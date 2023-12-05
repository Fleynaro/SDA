import { instance_of } from 'sda-bindings';
import {
  BaseSemanticsPropagator,
  ClassRepository,
  ClassResearcher,
  ConstConditionRepository,
  DataFlowCollector,
  DataFlowRepository,
  EventPipe,
  IRcodeContextSync,
  IRcodePcodeSync,
  IRcodeProgram,
  Image,
  ObjectAddedEvent,
  ObjectRemovedEvent,
  PcodeGraph,
  SemanticsRepository,
  SemanticsResearcher,
  SignatureRepository,
  SignatureResearcher,
  StandartSymbolTable,
  StructureRepository,
  StructureResearcher,
} from 'sda-core';
import { findPlatform } from './platform';

interface ImageInfo {
  image: Image;
  pcodeGraph: PcodeGraph;
  ircodeProgram: IRcodeProgram;
  ctxSync: IRcodeContextSync;
  pcodeSync: IRcodePcodeSync;
  researchers: {
    dataFlowRepo: DataFlowRepository;
    dataFlowResearcher: DataFlowCollector;
    signatureRepo: SignatureRepository;
    signatureResearcher: SignatureResearcher;
    constConditionRepo: ConstConditionRepository;
    structureRepo: StructureRepository;
    structureResearcher: StructureResearcher;
    classRepo: ClassRepository;
    classResearcher: ClassResearcher;
    semanticsRepo: SemanticsRepository;
    semanticsResearcher: SemanticsResearcher;
  };
}

const images: { [id: number]: ImageInfo } = {};
const programToImageInfo: { [id: number]: ImageInfo } = {};

export const getImageInfo = (image: Image): ImageInfo => {
  const info = images[image.hashId];
  if (info) {
    return info;
  }
  throw new Error(`Image ${image.name} not found`);
};

export const getImageInfoByProgram = (program: IRcodeProgram): ImageInfo => {
  const info = programToImageInfo[program.hashId];
  if (info) {
    return info;
  }
  throw new Error(`Program ${program.hashId} not found`);
};

export const initImageRepo = (eventPipe: EventPipe) => {
  eventPipe.subscribe((event) => {
    if (instance_of(event, ObjectAddedEvent)) {
      const e = event as ObjectAddedEvent;
      if (instance_of(e.object, Image)) {
        const image = e.object as Image;
        const ctxPipe = image.context.eventPipe;
        const imagePipe = EventPipe.New(`${image.id}-pipe`);
        ctxPipe.connect(imagePipe);
        const symbolTable = StandartSymbolTable.New(image.context, `${image.id}-table`);
        const pcodeGraph = PcodeGraph.New(imagePipe, image.context.platform);
        const ircodeProgram = IRcodeProgram.New(pcodeGraph, symbolTable);
        const callingConv = findPlatform('x86-64').callingConventions[0];
        // ir-code<->context sync
        const ctxSync = IRcodeContextSync.New(ircodeProgram, symbolTable, callingConv);
        imagePipe.connect(ctxSync.eventPipe);
        // p-code->ir-code sync
        const pcodeSync = IRcodePcodeSync.New(ircodeProgram);
        imagePipe.connect(pcodeSync.eventPipe);
        // data flow
        const dataFlowRepo = DataFlowRepository.New(imagePipe);
        const dataFlowResearcher = DataFlowCollector.New(ircodeProgram, dataFlowRepo);
        imagePipe.connect(dataFlowResearcher.eventPipe);
        // signature
        const signatureRepo = SignatureRepository.New();
        const signatureResearcher = SignatureResearcher.New(
          ircodeProgram,
          callingConv,
          signatureRepo,
        );
        imagePipe.connect(signatureResearcher.eventPipe);
        // constant condition
        const constConditionRepo = ConstConditionRepository.New(ircodeProgram);
        imagePipe.connect(constConditionRepo.eventPipe);
        // structure
        const structureRepo = StructureRepository.New(imagePipe);
        const structureResearcher = StructureResearcher.New(
          ircodeProgram,
          structureRepo,
          dataFlowRepo,
          constConditionRepo,
        );
        imagePipe.connect(structureResearcher.eventPipe);
        // class
        const classRepo = ClassRepository.New(imagePipe);
        const classResearcher = ClassResearcher.New(ircodeProgram, classRepo, structureRepo);
        imagePipe.connect(classResearcher.eventPipe);
        // semantics
        const semanticsRepo = SemanticsRepository.New(imagePipe);
        const semanticsResearcher = SemanticsResearcher.New(
          ircodeProgram,
          semanticsRepo,
          classRepo,
          dataFlowRepo,
        );
        const semanticsPropagator = BaseSemanticsPropagator.New(
          ircodeProgram,
          semanticsRepo,
          dataFlowRepo,
        );
        semanticsResearcher.addPropagator(semanticsPropagator);
        imagePipe.connect(semanticsResearcher.eventPipe);

        const imageInfo = {
          image: image,
          pcodeGraph: pcodeGraph,
          ircodeProgram,
          ctxSync,
          pcodeSync,
          researchers: {
            dataFlowRepo,
            dataFlowResearcher,
            signatureRepo,
            signatureResearcher,
            constConditionRepo,
            structureRepo,
            structureResearcher,
            classRepo,
            classResearcher,
            semanticsRepo,
            semanticsResearcher,
          },
        };
        images[image.hashId] = imageInfo;
        programToImageInfo[ircodeProgram.hashId] = imageInfo;
      }
    } else if (instance_of(event, ObjectRemovedEvent)) {
      const e = event as ObjectRemovedEvent;
      if (instance_of(e.object, Image)) {
        const image = e.object as Image;
        delete images[image.hashId];
      }
    }
  });
};
