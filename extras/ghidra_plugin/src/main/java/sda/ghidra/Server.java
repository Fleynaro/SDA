package sda.ghidra;

import org.apache.thrift.TMultiplexedProcessor;
import org.apache.thrift.server.TServer;
import org.apache.thrift.server.TSimpleServer;
import org.apache.thrift.transport.TServerSocket;
import org.apache.thrift.transport.TServerTransport;
import sda.Sda;
import sda.ghidra.packet.DataSyncPacketManagerService;
import sda.util.DebugConsole;

public class Server {
    private Sda sda;
    private TMultiplexedProcessor processor;
    private Thread workingThread;
    private TServer server;
    private int port = 9090;

    public Server(Sda sda, int port)
    {
        this.sda = sda;
        this.port = port;
        createMultiplexedProcessor();
        createWorkingThread();
    }

    public void start()
    {
        workingThread.start();
    }

    public void stop()
    {
        server.stop();
    }

    private void createMultiplexedProcessor()
    {
        processor = new TMultiplexedProcessor();
        processor.registerProcessor(
                "DataSyncPacketManager",
                new DataSyncPacketManagerService.Processor(new DataSyncPacketManagerServiceImpl(sda.getSync())));
    }

    private void createWorkingThread()
    {
        Runnable server = new Runnable() {
            public void run() {
                work();
            }
        };

        workingThread = new Thread(server);
    }

    private void work() {
        try {
            TServerTransport serverTransport = new TServerSocket(port);
            server = new TSimpleServer(new TServer.Args(serverTransport).processor(processor));
            DebugConsole.info(this, "Starting the simple server...");
            server.serve();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}