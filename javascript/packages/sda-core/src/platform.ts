import m from './module';
import { SignatureDataType } from './data-type';
import { Hash, IIdentifiable } from './utils';

export declare abstract class Platform implements IIdentifiable {
  readonly hashId: Hash;
  readonly className: string;
  readonly name: string;
  readonly pointerSize: number;
  readonly registerRepository: RegisterRepository;
  readonly callingConventions: CallingConvention[];

  getPcodeDecoder(): PcodeDecoder;

  getInstructionDecoder(): InstructionDecoder;

  static Get(hashId: Hash): Platform;
}

export namespace Register {
  export enum Type {
    Virtual,
    Generic,
    StackPointer,
    InstructionPointer,
    Flag,
    Vector,
  }
}

export declare abstract class RegisterRepository implements IIdentifiable {
  readonly hashId: Hash;
  readonly className: string;

  getRegisterName(regId: number): string;

  getRegisterId(regName: string): number;

  getRegisterType(regId: number): Register.Type;

  getRegisterFlagName(flagMask: number): string;

  getRegisterFlagIndex(flagName: string): number;

  static Get(hashId: Hash): RegisterRepository;
}

export declare abstract class PcodeDecoder implements IIdentifiable {
  readonly hashId: Hash;
  readonly className: string;
  readonly instructionLength: number;

  decode(offset: number, bytes: number[]): void;

  static Get(hashId: Hash): PcodeDecoder;
}

export namespace Instruction {
  export enum Type {
    None,
    ConditionalBranch,
    UnconditionalBranch,
  }

  export enum TokenType {
    Mneumonic,
    Register,
    Number,
    AddressAbs,
    AddressRel,
    Other,
  }

  export type Token = {
    type: TokenType;
    text: string;
  };
}

export declare abstract class InstructionDecoder implements IIdentifiable {
  readonly hashId: Hash;
  readonly className: string;
  decode(bytes: number[]): void;

  static Get(hashId: Hash): InstructionDecoder;
}

export namespace CallingConvention {
  export enum UseType {
    Read,
    Write,
  }

  export type Storage = {
    useType: UseType;
    registerId: number;
    offset: number;
  };

  export enum StorageType {
    None,
    Return,
    Parameter,
  }

  export type StorageInfo = {
    type: StorageType;
    paramIdx: number;
    isStoringFloat: boolean;
  };

  export type Map = {
    //[storage: CallingConvention.Storage]: CallingConvention.StorageInfo
  };
}

export declare abstract class CallingConvention implements IIdentifiable {
  readonly hashId: Hash;
  readonly className: string;
  readonly name: string;

  getStorages(signatureDt: SignatureDataType): CallingConvention.Map;

  static Get(hashId: Hash): CallingConvention;
}

export declare class CustomCallingConvention extends CallingConvention {
  static New(storages: CallingConvention.Map): CustomCallingConvention;
}

module.exports = {
  ...module.exports,
  CustomCallingConvention: m.CustomCallingConvention,
};
