import { SignatureDataType } from "./data-type";
import { Hash, IIdentifiable } from "./utils";

export abstract class Platform implements IIdentifiable {
    readonly hashId: Hash;
    readonly name: string;
    readonly pointerSize: number;
    readonly registerRepository: RegisterRepository;
    readonly callingConventions: CallingConvention[];

    getPcodeDecoder(): PcodeDecoder;

    getInstructionDecoder(): InstructionDecoder;

    static Get(hashId: Hash): Platform;
}

export namespace Register {
    type Type =
        "Virtual"               |
        "Generic"               |
        "StackPointer"          |
        "InstructionPointer"    |
        "Flag"                  |
        "Vector";
}

export abstract class RegisterRepository implements IIdentifiable {
    readonly hashId: Hash;

    getRegisterName(regId: number): string;

    getRegisterId(regName: string): number;

    getRegisterType(regId: number): Register.Type;

    getRegisterFlagName(flagMask: number): string;

    getRegisterFlagIndex(flagName: string): number;
    
    static Get(hashId: Hash): RegisterRepository;
}

export abstract class PcodeDecoder implements IIdentifiable {
    readonly hashId: Hash;
    readonly instructionLength: number;

    decode(offset: number, bytes: number[]): void;

    static Get(hashId: Hash): PcodeDecoder;
}

export abstract class InstructionDecoder implements IIdentifiable {
    readonly hashId: Hash;

    decode(bytes: number[]): void;

    static Get(hashId: Hash): InstructionDecoder;
}

export namespace CallingConvention {
    type Storage = {
        useType: "Read" | "Write";
        registerId: number;
        offset: number;
    }

    type StorageInfo = {
        type: "None" | "Return" | "Parameter";
        paramIdx: number;
        isStoringFloat: boolean;
    }

    type Map = {
        //[storage: CallingConvention.Storage]: CallingConvention.StorageInfo
    }
}

export abstract class CallingConvention implements IIdentifiable {
    readonly hashId: Hash;
    readonly name: string;

    getStorages(signatureDt: SignatureDataType): CallingConvention.Map;

    static Get(hashId: Hash): CallingConvention;
}

export class CustomCallingConvention extends CallingConvention {
    static New(storages: CallingConvention.Map): CustomCallingConvention;
}
