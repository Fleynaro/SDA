#include "AddressSpace.h"
#include <Manager/AddressSpaceManager.h>

fs::path CE::AddressSpace::getImagesDirectory() {
	return getAddrSpaceManager()->getProject()->getImagesDirectory() / fs::path(getName());
}
