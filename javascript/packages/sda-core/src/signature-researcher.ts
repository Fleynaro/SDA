import m from './module';
import { EventPipe } from './event';
import { IRcodeProgram } from './ir-code';
import { CallingConvention } from './platform';

export declare class SignatureRepository {
  readonly eventPipe: EventPipe;

  static New(): SignatureRepository;
}

export declare class SignatureResearcher {
  readonly eventPipe: EventPipe;

  static New(
    program: IRcodeProgram,
    callingConvention: CallingConvention,
    repository: SignatureRepository,
  ): SignatureResearcher;
}

module.exports = {
  ...module.exports,
  SignatureRepository: m.SignatureRepository,
  SignatureResearcher: m.SignatureResearcher,
};
