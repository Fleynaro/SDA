import { PcodeController, PcodeObjectId } from 'api/p-code';
import { ObjectId, Offset } from 'api/common';
import { invokerFactory } from '../utils';

const invoke = invokerFactory('Pcode');

const PcodeControllerImpl: PcodeController = {
  getGraphIdByImage: (id: ObjectId) => invoke('getGraphIdByImage', id),

  getBlockAt: (graphId: ObjectId, offset: Offset, halfInterval: boolean) =>
    invoke('getBlockAt', graphId, offset, halfInterval),

  getFunctionGraphAt: (graphId: ObjectId, entryOffset: Offset) =>
    invoke('getFunctionGraphAt', graphId, entryOffset),

  getPcodeTokenizedText: (contextId: ObjectId, funcGraphId: PcodeObjectId) =>
    invoke('getPcodeTokenizedText', contextId, funcGraphId),
};

export default PcodeControllerImpl;
