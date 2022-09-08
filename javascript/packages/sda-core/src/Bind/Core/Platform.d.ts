declare module sda {
    abstract class Platform {
        readonly name: string;
        readonly pointerSize: number;
        readonly registerRepository: RegisterRepository;
        readonly callingConventions: CallingConvention[];

        getPcodeDecoder(): PcodeDecoder;

        getInstructionDecoder(): InstructionDecoder;
    }

    abstract class RegisterRepository {

    }

    abstract class PcodeDecoder {

    }

    abstract class InstructionDecoder {

    }

    abstract class CallingConvention {
        readonly name: string;
    }
}