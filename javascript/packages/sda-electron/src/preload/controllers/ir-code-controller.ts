import { IRcodeController, IRcodeObjectId } from 'api/ir-code';
import { ObjectId, Offset } from 'api/common';
import { invokerFactory } from '../utils';

const invoke = invokerFactory('IRcode');

const IRcodeControllerImpl: IRcodeController = {
  getProgramIdByImage: (id: ObjectId) => invoke('getProgramIdByImage', id),

  getFunctionAt: (programId: ObjectId, entryOffset: Offset) =>
    invoke('getFunctionAt', programId, entryOffset),

  getIRcodeTokenizedText: (contextId: ObjectId, functionId: IRcodeObjectId) =>
    invoke('getIRcodeTokenizedText', contextId, functionId),
};

export default IRcodeControllerImpl;
