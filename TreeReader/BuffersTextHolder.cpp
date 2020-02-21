#include "BuffersTextHolder.h"

#include <algorithm>

namespace TreeReader
{
   using namespace std;

   pair<wchar_t*, size_t> BuffersTextHolderReader::ReadLine(wistream& stream)
   {
      bool skipNewLines = true;

      wchar_t* line = PosInBuffer;
      while (true)
      {
         if (PosInBuffer >= BufferEnd)
         {
            const size_t amountReadSoFar = PosInBuffer - line;
            const size_t bufferSize = max(size_t(64 * 1024), amountReadSoFar * 2);

            Holder->TextBuffers.emplace_back(make_shared<BuffersTextHolder::Buffer>());
            auto buffer = Holder->TextBuffers.back();
            buffer->resize(bufferSize);
            if (amountReadSoFar > 0)
               memcpy(buffer->data(), line, amountReadSoFar);

            line = buffer->data();
            BufferEnd = line + buffer->size();
            PosInBuffer = line + amountReadSoFar;

            stream.read(PosInBuffer, BufferEnd - PosInBuffer);
            const auto readAmount = stream.gcount();
            if (readAmount <= 0)
               break;
            BufferEnd = PosInBuffer + readAmount;
         }

         if (skipNewLines)
         {
            if (*PosInBuffer != '\n' && *PosInBuffer != '\r')
            {
               line = PosInBuffer;
               skipNewLines = false;
            }
         }
         else
         {
            if (*PosInBuffer == '\n' || *PosInBuffer == '\r')
            {
               *PosInBuffer = 0;
               PosInBuffer++;
               break;
            }
         }

         ++PosInBuffer;
      }

      return make_pair(line, PosInBuffer - line);
   }
}
