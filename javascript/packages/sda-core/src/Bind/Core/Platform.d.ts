declare module sda {
    abstract class Platform {
        readonly name: string;
        readonly pointerSize: number;
        readonly registerRepository: RegisterRepository;
        readonly callingConventions: CallingConvention[];

        getPcodeDecoder(): PcodeDecoder;

        getInstructionDecoder(): InstructionDecoder;
    }

    module Register {
        type Type =
            "Virtual"               |
            "Generic"               |
            "StackPointer"          |
            "InstructionPointer"    |
            "Flag"                  |
            "Vector";
    }

    abstract class RegisterRepository {
        getRegisterName(regId: number): string;

        getRegisterId(regName: string): number;

        getRegisterType(regId: number): Register.Type;

        getRegisterFlagName(flagMask: number): string;

        getRegisterFlagIndex(flagName: string): number;
    }

    abstract class PcodeDecoder {
        readonly instructionLength: number;

        decode(offset: number, bytes: number[]): void;
    }

    abstract class InstructionDecoder {
        decode(bytes: number[]): void;
    }

    module CallingConvention {
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
            [storage: CallingConvention.Storage]: CallingConvention.StorageInfo
        }
    }

    abstract class CallingConvention {
        readonly name: string;

        getStorages(signatureDt: SignatureDataType): CallingConvention.Map;
    }

    class CustomCallingConvention extends CallingConvention {
        static New(storages: CallingConvention.Map): CustomCallingConvention;
    }
}