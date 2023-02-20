using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO.MemoryMappedFiles;

namespace MemoryVisualizer
{
    /*
     * Used for IPC comunication whose responsibility is only to read the shared memory.
     * 2-way transmission is not yet possible. Sockets are probably better way to do that
     */
    class SharedMemoryLooker
    {
        public static MemoryMappedViewStream? ReadFromSharedMemoryLocation()
        {
            MemoryMappedFile file = MemoryMappedFile.OpenExisting(sharedMemoryName);
            if (file == null)
                return null;

            MemoryMappedViewStream stream = file.CreateViewStream(0, sharedMemorySize, MemoryMappedFileAccess.Read);

            return stream;
        }

        //TODO: This should be read from a settings file
        const string sharedMemoryName = "TestMemoryMap";
        const uint sharedMemorySize = 10 * 1024 * 1024;
    }
}
