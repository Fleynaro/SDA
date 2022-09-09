declare module sda {
    abstract class PcodeVarnode {
        readonly size: number;
        readonly isRegister: boolean;
    }

    class PcodeInstruction {
        readonly id: string;
        readonly input0: PcodeVarnode;
        readonly input1: PcodeVarnode;
        readonly output: PcodeVarnode;
        readonly offset: number;
    }

    class PcodeParser {
        static Parse(text: string, regRepo: RegisterRepository): PcodeInstruction[];
    }

    class PcodePrinter extends AbstractPrinter {
        printInstruction(pcode: PcodeInstruction): void;

        printVarnode(varnode: PcodeVarnode): void;

        static New(regRepo: RegisterRepository): PcodePrinter;

        static Print(instruction: PcodeInstruction, regRepo: RegisterRepository): string;
    }
}