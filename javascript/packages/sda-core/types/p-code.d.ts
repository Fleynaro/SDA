import { RegisterRepository } from "./platform";
import { AbstractPrinter } from "./utils";

export abstract class PcodeVarnode {
    readonly size: number;
    readonly isRegister: boolean;
}

export class PcodeInstruction {
    readonly id: string;
    readonly input0: PcodeVarnode;
    readonly input1: PcodeVarnode;
    readonly output: PcodeVarnode;
    readonly offset: number;
}

export class PcodeParser {
    static Parse(text: string, regRepo: RegisterRepository): PcodeInstruction[];
}

export class PcodePrinter extends AbstractPrinter {
    printInstruction(pcode: PcodeInstruction): void;

    printVarnode(varnode: PcodeVarnode): void;

    static New(regRepo: RegisterRepository): PcodePrinter;

    static Print(instruction: PcodeInstruction, regRepo: RegisterRepository): string;
}