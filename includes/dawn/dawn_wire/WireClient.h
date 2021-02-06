// Copyright 2019 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DAWNWIRE_WIRECLIENT_H_
#define DAWNWIRE_WIRECLIENT_H_

#include "dawn/dawn_proc_table.h"
#include "dawn_wire/Wire.h"

#include <memory>
#include <vector>

namespace dawn_wire {

    namespace client {
        class Client;
        class MemoryTransferService;

        DAWN_WIRE_EXPORT const DawnProcTable& GetProcs();
    }  // namespace client

    struct ReservedTexture {
        WGPUTexture texture;
        uint32_t id;
        uint32_t generation;
        uint32_t deviceId;
        uint32_t deviceGeneration;
    };

    struct DAWN_WIRE_EXPORT WireClientDescriptor {
        CommandSerializer* serializer;
        client::MemoryTransferService* memoryTransferService = nullptr;
    };

    class DAWN_WIRE_EXPORT WireClient : public CommandHandler {
      public:
        WireClient(const WireClientDescriptor& descriptor);
        ~WireClient() override;

        WGPUDevice GetDevice() const;
        const volatile char* HandleCommands(const volatile char* commands,
                                            size_t size) override final;

        ReservedTexture ReserveTexture(WGPUDevice device);

        // Disconnects the client.
        // Commands allocated after this point will not be sent.
        void Disconnect();

      private:
        std::unique_ptr<client::Client> mImpl;
    };

    namespace client {
        class DAWN_WIRE_EXPORT MemoryTransferService {
          public:
            MemoryTransferService();
            virtual ~MemoryTransferService();

            class ReadHandle;
            class WriteHandle;

            // Create a handle for reading server data.
            // This may fail and return nullptr.
            virtual ReadHandle* CreateReadHandle(size_t) = 0;

            // Create a handle for writing server data.
            // This may fail and return nullptr.
            virtual WriteHandle* CreateWriteHandle(size_t) = 0;

            // Imported memory implementation needs to override these to create Read/Write
            // handles associated with a particular buffer. The client should receive a file
            // descriptor for the buffer out-of-band.
            virtual ReadHandle* CreateReadHandle(WGPUBuffer, uint64_t offset, size_t size);
            virtual WriteHandle* CreateWriteHandle(WGPUBuffer, uint64_t offset, size_t size);

            class DAWN_WIRE_EXPORT ReadHandle {
              public:
                ReadHandle();
                virtual ~ReadHandle();

                // Get the required serialization size for SerializeCreate
                virtual size_t SerializeCreateSize() = 0;

                // Serialize the handle into |serializePointer| so it can be received by the server.
                virtual void SerializeCreate(void* serializePointer) = 0;

                // Load initial data and open the handle for reading.
                // This function takes in the serialized result of
                // server::MemoryTransferService::ReadHandle::SerializeInitialData.
                // This function should write to |data| and |dataLength| the pointer and size of the
                // mapped data for reading. It must live at least until the ReadHandle is
                // destructed.
                virtual bool DeserializeInitialData(const void* deserializePointer,
                                                    size_t deserializeSize,
                                                    const void** data,
                                                    size_t* dataLength) = 0;

              private:
                ReadHandle(const ReadHandle&) = delete;
                ReadHandle& operator=(const ReadHandle&) = delete;
            };

            class DAWN_WIRE_EXPORT WriteHandle {
              public:
                WriteHandle();
                virtual ~WriteHandle();

                // Get the required serialization size for SerializeCreate
                virtual size_t SerializeCreateSize() = 0;

                // Serialize the handle into |serializePointer| so it can be received by the server.
                virtual void SerializeCreate(void* serializePointer) = 0;

                // Open the handle for reading. The data returned should be zero-initialized.
                // The data returned must live at least until the WriteHandle is destructed.
                // On failure, the pointer returned should be null.
                virtual std::pair<void*, size_t> Open() = 0;

                // Get the required serialization size for SerializeFlush
                virtual size_t SerializeFlushSize() = 0;

                // Flush writes to the handle. This should serialize info to send updates to the
                // server.
                virtual void SerializeFlush(void* serializePointer) = 0;

              private:
                WriteHandle(const WriteHandle&) = delete;
                WriteHandle& operator=(const WriteHandle&) = delete;
            };

          private:
            MemoryTransferService(const MemoryTransferService&) = delete;
            MemoryTransferService& operator=(const MemoryTransferService&) = delete;
        };

        // Backdoor to get the order of the ProcMap for testing
        DAWN_WIRE_EXPORT std::vector<const char*> GetProcMapNamesForTesting();
    }  // namespace client
}  // namespace dawn_wire

#endif  // DAWNWIRE_WIRECLIENT_H_