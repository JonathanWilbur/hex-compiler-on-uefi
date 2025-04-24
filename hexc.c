#include <efi.h>

EFI_GUID gEfiLoadedImageProtocolGuid = { 0x5B1B31A1, 0x9562, 0x11D2, { 0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }};
EFI_GUID gEfiSimpleFileSystemProtocolGuid = { 0x964E5B22, 0x6459, 0x11D2, { 0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }};

#define FileSystemProtocol gEfiSimpleFileSystemProtocolGuid
#define LoadedImageProtocol gEfiLoadedImageProtocolGuid

CHAR16 *StrChr(const CHAR16 *s, CHAR16 c) {
    while (*s) {
        if (*s == c) return (CHAR16 *)s;
        s++;
    }
    return NULL;
}

CHAR16 *StrTok(CHAR16 *str, const CHAR16 *delim, CHAR16 **saveptr) {
    CHAR16 *start, *end;

    if (str)
        start = str;
    else if (*saveptr)
        start = *saveptr;
    else
        return NULL;

    // Skip leading delimiters
    while (*start && StrChr(delim, *start)) start++;
    if (*start == L'\0') {
        *saveptr = NULL;
        return NULL;
    }

    end = start;
    while (*end && !StrChr(delim, *end)) end++;

    if (*end) {
        *end = L'\0';
        *saveptr = end + 1;
    } else {
        *saveptr = NULL;
    }

    return start;
}

SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut;
#define Print(s) ConOut->OutputString(ConOut, s)

EFI_STATUS
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_LOADED_IMAGE *LoadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    EFI_FILE_PROTOCOL *RootDir, *InputFile, *OutputFile;
    CHAR16 *InputFileName, *OutputFileName;
    ConOut = SystemTable->ConOut;
    Print(L"Compiling hex...\n");

    SystemTable->BootServices->HandleProtocol(ImageHandle, &LoadedImageProtocol, (void**)&LoadedImage);

    UINTN Argc = 0;
    CHAR16 *Argv[10]; // max 10 args
    CHAR16 *token, *next;
    token = StrTok(LoadedImage->LoadOptions, L" ", &next);
    while (token && Argc < 10) {
        Argv[Argc++] = token;
        token = StrTok(NULL, L" ", &next);
    }

    if (Argc != 3) {
        // Print(L"Usage: hex2bin <input_file> <output_file>\n");
        return EFI_INVALID_PARAMETER;
    }

    InputFileName = Argv[1];
    OutputFileName = Argv[2];

    // Get volume and open root directory
    SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &FileSystemProtocol, (void**)&FileSystem);
    FileSystem->OpenVolume(FileSystem, &RootDir);

    // Open input file
    if (RootDir->Open(RootDir, &InputFile, InputFileName, EFI_FILE_MODE_READ, 0) != EFI_SUCCESS) {
        // Print(L"Failed to open input file: %s\n", InputFileName);
        return EFI_NOT_FOUND;
    }

    // Create or overwrite output file
    if (RootDir->Open(RootDir, &OutputFile, OutputFileName,
                      EFI_FILE_MODE_CREATE | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ,
                      0) != EFI_SUCCESS) {
        // Print(L"Failed to open output file: %s\n", OutputFileName);
        InputFile->Close(InputFile);
        return EFI_DEVICE_ERROR;
    }

    // Allocate buffer and read file content
    UINTN BufSize = 10000000; // 10MB is larger than any file any human can read.
    CHAR8 *ReadBuf;
    CHAR8 *WriteBuf;
    
    // TODO: Handle errors
    SystemTable->BootServices->AllocatePool(EfiLoaderData, BufSize, (void **)&ReadBuf);
    SystemTable->BootServices->AllocatePool(EfiLoaderData, BufSize / 2, (void **)&WriteBuf);
    UINTN ReadSize = BufSize;
    InputFile->Read(InputFile, &ReadSize, ReadBuf);

    UINTN WriteLen = 0;
    for (UINTN i = 0; i < ReadSize;) {
        // Skip whitespace
        while (i < ReadSize && (ReadBuf[i] == ' ' || ReadBuf[i] == '\r' || ReadBuf[i] == '\n' || ReadBuf[i] == '\t'))
            i++;

        // Skip comments
        if (i < ReadSize && ReadBuf[i] == '#') {
            while (i < ReadSize && ReadBuf[i] != '\n')
                i++;
            continue;
        }

        if (i + 1 >= ReadSize) break;

        // Convert hex pair to byte
        CHAR8 hi = ReadBuf[i++];
        CHAR8 lo = ReadBuf[i++];

        UINT8 byte = 0;
        if (hi >= '0' && hi <= '9') byte |= (hi - '0') << 4;
        else if (hi >= 'a' && hi <= 'f') byte |= (hi - 'a' + 10) << 4;
        else if (hi >= 'A' && hi <= 'F') byte |= (hi - 'A' + 10) << 4;
        else continue;

        if (lo >= '0' && lo <= '9') byte |= (lo - '0');
        else if (lo >= 'a' && lo <= 'f') byte |= (lo - 'a' + 10);
        else if (lo >= 'A' && lo <= 'F') byte |= (lo - 'A' + 10);
        else continue;

        WriteBuf[WriteLen++] = byte;
    }
    OutputFile->Write(OutputFile, &WriteLen, WriteBuf);
    InputFile->Close(InputFile);
    OutputFile->Close(OutputFile);
    RootDir->Close(RootDir);
    SystemTable->BootServices->FreePool(ReadBuf);
    SystemTable->BootServices->FreePool(WriteBuf);

    // Print(L"Wrote %u bytes\n", WriteLen);
    return EFI_SUCCESS;
}
