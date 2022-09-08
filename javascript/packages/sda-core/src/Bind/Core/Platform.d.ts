declare module sda {
    abstract class Platform {
        readonly name: string;
        readonly pointerSize: number;
        readonly registerRepository: RegisterRepository;
        readonly callingConventions: CallingConvention[];

        getPcodeDecoder(): PcodeDecoder;

        getInstructionDecoder(): InstructionDecoder;
    }

    type RegisterType =
        "Virtual"               |
        "Generic"               |
        "StackPointer"          |
        "InstructionPointer"    |
        "Flag"                  |
        "Vector";

    abstract class RegisterRepository {
        getRegisterName(regId: number): string;

        getRegisterId(regName: string): number;

        getRegisterType(regId: number): RegisterType;

        getRegisterFlagName(flagMask: number): string;

        getRegisterFlagIndex(flagName: string): number;
    }

    abstract class PcodeDecoder {

    }

    abstract class InstructionDecoder {

    }

    abstract class CallingConvention {
        readonly name: string;
    }

    class CustomCallingConvention extends CallingConvention {
        static New(): CustomCallingConvention;
    }
}