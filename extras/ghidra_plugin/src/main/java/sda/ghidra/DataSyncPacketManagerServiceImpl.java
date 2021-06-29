package sda.ghidra;

import org.apache.thrift.TException;
import sda.ghidra.packet.DataSyncPacketManagerService;
import sda.ghidra.packet.SDataFullSyncPacket;
import sda.ghidra.packet.SDataLightSyncPacket;
import sda.sync.Sync;

class DataSyncPacketManagerServiceImpl implements DataSyncPacketManagerService.Iface
{
    private Sync sync;

    public DataSyncPacketManagerServiceImpl(Sync sync) {
        this.sync = sync;
    }

    @Override
    public SDataLightSyncPacket recieveLightSyncPacket() throws TException {
        return null;
    }

    @Override
    public void sendLightSyncPacket(SDataLightSyncPacket packet) throws TException {
    }

    @Override
    public SDataFullSyncPacket recieveFullSyncPacket() throws TException {
        return sync.buildDataFullSyncPacket();
    }

    @Override
    public void sendFullSyncPacket(SDataFullSyncPacket packet) throws TException {
        sync.loadDataFullSyncPacket(packet);
    }
}
