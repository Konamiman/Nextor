/*
 * Access to the Nextor persistent storage, and to the data that Nextor
 * keeps at the very beginning of it.
 *
 * These routines never terminate the program nor print anything: they
 * return the error code of the _PSOPS function call, so that each program
 * can handle the errors the way it prefers. The ones a caller will usually
 * want to tell apart are:
 *
 *   _IBDOS: this version of Nextor doesn't have the _PSOPS function call.
 *   _IDRVR: there's no persistent storage available.
 *   _NOFIL: the storage is file based and the file doesn't exist yet
 *           (PsLoadData still leaves a fresh set of data in the buffer,
 *           and a later PsSaveData creates the file).
 *
 * See "Persistent storage" in the Nextor Programmers Reference.
 */

#include "types.h"
#include "asmcall.h"
#include "dos_functions.h"
#include "dos_errors.h"
#include "persistent_storage.h"

static Z80_registers psRegs;

static const byte psFreshData[PSD_MIN_SIZE] = PSD_FRESH_DATA;

static byte PsCall(byte operation, byte* buffer, uint firstSector, byte sectorCount)
{
    psRegs.Bytes.A = operation;
    psRegs.Words.HL = (int)buffer;
    psRegs.Words.DE = firstSector;
    psRegs.Bytes.B = sectorCount;
    DosCall(_PSOPS, &psRegs, REGS_ALL, REGS_ALL);
    return psRegs.Bytes.A;
}

byte PsGetInfo(persistentStorageInfo* info)
{
    return PsCall(PSOPS_GET_INFO, (byte*)info, 0, 0);
}

byte PsRead(byte* buffer, uint firstSector, byte sectorCount)
{
    return PsCall(PSOPS_READ, buffer, firstSector, sectorCount);
}

byte PsWrite(byte* buffer, uint firstSector, byte sectorCount)
{
    return PsCall(PSOPS_WRITE, buffer, firstSector, sectorCount);
}

byte PsSectorsThatFit(persistentStorageInfo* info, uint bufferSize, uint* availableBytes)
{
    byte count;
    uint total;

    count = 0;
    total = 0;
    while(count < info->sectorCount && count < 255 &&
          total + info->sectorSize <= bufferSize) {
        count++;
        total += info->sectorSize;
    }

    *availableBytes = total;
    return count;
}

bool PsDataIsValid(byte* buffer, uint availableBytes)
{
    uint size;
    uint i;
    byte sum;

    for(i = 0; i < 7; i++) {
        if(buffer[i] != psFreshData[i]) {
            return false;
        }
    }

    size = buffer[PSD_SIZE] + (buffer[PSD_SIZE + 1] << 8);
    if(size < PSD_MIN_SIZE || size > availableBytes) {
        return false;
    }

    sum = 0;
    for(i = 0; i < size; i++) {
        sum += buffer[i];
    }
    return sum == 0;
}

void PsInitData(byte* buffer, uint bufferSize)
{
    uint i;
    uint count;

    for(i = 0; i < bufferSize; i++) {
        buffer[i] = 0;
    }

    /* A buffer of less than PSD_MIN_SIZE bytes can't hold a valid set of
       data, but write what fits rather than past the end of it. */
    count = bufferSize < PSD_MIN_SIZE ? bufferSize : PSD_MIN_SIZE;
    for(i = 0; i < count; i++) {
        buffer[i] = psFreshData[i];
    }
}

void PsSetChecksum(byte* buffer)
{
    uint size;
    uint i;
    byte sum;

    size = buffer[PSD_SIZE] + (buffer[PSD_SIZE + 1] << 8);

    sum = 0;
    for(i = 0; i < size - 1; i++) {
        sum += buffer[i];
    }
    buffer[size - 1] = (byte)(0 - sum);
}

byte PsLoadData(byte* buffer, uint bufferSize, persistentStorageInfo* info, byte* sectorsUsed)
{
    byte error;
    uint availableBytes;

    *sectorsUsed = 0;

    error = PsGetInfo(info);
    if(error != 0) {
        return error;
    }

    *sectorsUsed = PsSectorsThatFit(info, bufferSize, &availableBytes);

    error = PsRead(buffer, 0, *sectorsUsed);
    if(error == 0 && PsDataIsValid(buffer, availableBytes)) {
        return 0;
    }

    /* Either the storage couldn't be read, or what it holds isn't valid
       data: leave a fresh, empty set of data in the buffer either way. */
    PsInitData(buffer, bufferSize);
    return error;
}

byte PsSaveData(byte* buffer, byte sectorsUsed)
{
    PsSetChecksum(buffer);
    return PsWrite(buffer, 0, sectorsUsed);
}
