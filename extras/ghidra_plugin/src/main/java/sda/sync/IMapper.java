package sda.sync;

import sda.ghidra.packet.*;

/*

1) есть sync класс со всеми там мапперами
2) абстрактного маппера нет, есть конкретные только
3) есть синхро контекст, где ссылка на синхро фулл пакет
4) мы должны пройтись итератором по менеджерам функций, типов и т.д. и применить для них сериализацию мапперами, юзая контекст. Делать это классом sync
5) сералиацию делать также как и в с++.
 */

public interface IMapper {

    public void load(SDataFullSyncPacket dataPacket);

}

