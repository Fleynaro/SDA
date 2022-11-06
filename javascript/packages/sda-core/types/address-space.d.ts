import { Context } from "./context";
import { ContextObject } from "./object";
import { Image } from "./image";

export abstract class AddressSpace extends ContextObject {
    images: Image[];

    static New(context: Context, name: string): AddressSpace;
}