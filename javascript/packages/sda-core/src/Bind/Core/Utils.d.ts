declare module sda {
    interface ISerializable {
        serialize(): object;

        deserialize(data: object): void;
    }
}