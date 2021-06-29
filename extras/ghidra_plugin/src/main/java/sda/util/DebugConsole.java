package sda.util;

import ghidra.util.Msg;

public class DebugConsole {
    private static String senderName = "SDA";

    private DebugConsole() {
    }

    private static String getPrefix() {
        return senderName + ": ";
    }

    public static void info(Object originator, Object message) {
        Msg.info(originator, getPrefix() + message);
    }

    public static void warn(Object originator, Object message) {
        Msg.warn(originator, getPrefix() + message);
    }

    public static void error(Object originator, Object message) {
        Msg.error(originator, getPrefix() + message);
    }
}
