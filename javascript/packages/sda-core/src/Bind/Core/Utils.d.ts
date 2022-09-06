declare module sda_core {
    interface ISerializable {
        serialize(): object;

        deserialize(data: object): void;
    }
}