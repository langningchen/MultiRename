#include <windows.h>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <vector>
#include <string>
#include <set>
#include <conio.h>

struct DIRECTORY_INFO
{
    std::string Name;
    SYSTEMTIME CreationTime;
    SYSTEMTIME LastWriteTime;
    SYSTEMTIME LastAccessTime;
};
bool operator==(DIRECTORY_INFO a, DIRECTORY_INFO b)
{
    if (a.Name != b.Name ||
        memcmp(&a.CreationTime, &b.CreationTime, sizeof(SYSTEMTIME)) != 0 ||
        memcmp(&a.LastWriteTime, &b.LastWriteTime, sizeof(SYSTEMTIME)) != 0 ||
        memcmp(&a.LastAccessTime, &b.LastAccessTime, sizeof(SYSTEMTIME)) != 0)
        return false;
    return true;
}
struct FILE_INFO
{
    std::string FullName;      // File.Extension
    std::string Name;          // File
    std::string Extension;     // Extension
    std::string Directory;     // C:/Folder
    std::string FullPath;      // C:/Folder/File.Extension
    long Size;                 // 1024 Bytes
    SYSTEMTIME CreationTime;   // 2019-01-01 00:00:00
    SYSTEMTIME LastWriteTime;  // 2019-01-01 00:00:00
    SYSTEMTIME LastAccessTime; // 2019-01-01 00:00:00
};
bool operator==(FILE_INFO a, FILE_INFO b)
{
    return a.FullPath == b.FullPath;
}

std::string CurrentDir = "C:";
std::string Pattern = "[F]";
std::vector<FILE_INFO> FilesToRename;
std::vector<std::pair<int, std::pair<int, int>>> Indexes;

template <typename T>
size_t FindInVector(const std::vector<T> &Vector, const T &Value)
{
    for (size_t i = 0; i < Vector.size(); i++)
        if (Vector[i] == Value)
            return i;
    return std::string::npos;
}
std::string StringReplaceAll(std::string Input, std::string Find, std::string Replace)
{
    size_t FoundIndex = Input.find(Find);
    while (FoundIndex != std::string::npos)
    {
        Input.replace(FoundIndex, FoundIndex + Find.length() + 1, Replace);
        FoundIndex = Input.find(Find);
    }
    return Input;
}
void InitialConsole()
{
    HANDLE StandardOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD ConsoleMode = 0;
    GetConsoleMode(StandardOutput, &ConsoleMode);
    ConsoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(StandardOutput, ConsoleMode);
    CONSOLE_CURSOR_INFO CursorInfo;
    GetConsoleCursorInfo(StandardOutput, &CursorInfo);
    CursorInfo.bVisible = false;
    SetConsoleCursorInfo(StandardOutput, &CursorInfo);
}
std::vector<DIRECTORY_INFO> GetDirectoriesInDirectory(const std::string &Directory)
{
    std::vector<DIRECTORY_INFO> Directories;
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle = FindFirstFile((Directory + "/*.*").c_str(), &FindData);
    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                DIRECTORY_INFO DirectoryInfo;
                DirectoryInfo.Name = FindData.cFileName;
                if (DirectoryInfo.Name == "." || DirectoryInfo.Name == "..")
                    continue;
                FileTimeToSystemTime(&FindData.ftCreationTime, &DirectoryInfo.CreationTime);
                FileTimeToSystemTime(&FindData.ftLastWriteTime, &DirectoryInfo.LastWriteTime);
                FileTimeToSystemTime(&FindData.ftLastAccessTime, &DirectoryInfo.LastAccessTime);
                Directories.push_back(DirectoryInfo);
            }
        } while (::FindNextFile(FindHandle, &FindData));
        FindClose(FindHandle);
    }
    return Directories;
}
std::vector<FILE_INFO> GetFilesInDirectory(const std::string &Directory)
{
    std::vector<FILE_INFO> Files;
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle = FindFirstFile((Directory + "/*.*").c_str(), &FindData);
    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                FILE_INFO FileInfo;
                FileInfo.FullName = FindData.cFileName;
                FileInfo.Name = FindData.cFileName;
                FileInfo.Extension = "";
                size_t DotPos = FileInfo.Name.rfind('.');
                if (DotPos != std::string::npos)
                {
                    FileInfo.Name = FileInfo.FullName.substr(0, DotPos);
                    FileInfo.Extension = FileInfo.FullName.substr(DotPos + 1);
                }
                FileInfo.Directory = Directory;
                FileInfo.FullPath = Directory + "/" + FileInfo.FullName;
                FileInfo.Size = FindData.nFileSizeLow;
                FileTimeToSystemTime(&FindData.ftCreationTime, &FileInfo.CreationTime);
                FileTimeToSystemTime(&FindData.ftLastWriteTime, &FileInfo.LastWriteTime);
                FileTimeToSystemTime(&FindData.ftLastAccessTime, &FileInfo.LastAccessTime);
                Files.push_back(FileInfo);
            }
        } while (::FindNextFile(FindHandle, &FindData));
        FindClose(FindHandle);
    }
    return Files;
}
void SelectFiles()
{
    size_t CurrentCursor = 0;
    while (true)
    {
        std::vector<DIRECTORY_INFO> Directories = GetDirectoriesInDirectory(CurrentDir);
        std::vector<FILE_INFO> Files = GetFilesInDirectory(CurrentDir);
        std::cout << "\033[H\033[J\033[33m" << CurrentDir << "\033[0m" << std::endl
                  << std::endl;
        for (const auto &Directory : Directories)
        {
            if (FindInVector(Directories, Directory) == CurrentCursor)
                std::cout << "\033[7m";
            std::cout << "\033[34m" << Directory.Name << "\033[0m" << std::endl;
        }
        for (const auto &File : Files)
        {
            if (FindInVector(Files, File) == CurrentCursor - Directories.size())
                std::cout << "\033[7m";
            if (FindInVector(FilesToRename, File) != std::string::npos)
                std::cout << "\033[32m";
            std::cout << File.FullName << "\033[0m" << std::endl;
        }
        int Input = _getch();
        if (Input == 0 || Input == 224)
        {
            Input = _getch();
            if (Input == 72 && CurrentCursor > 0)
                CurrentCursor--;
            else if (Input == 80 && CurrentCursor < Directories.size() + Files.size() - 1)
                CurrentCursor++;
        }
        else if (Input == 8)
        {
            size_t SlashPos = CurrentDir.rfind('/');
            if (SlashPos != std::string::npos)
            {
                CurrentDir = CurrentDir.substr(0, SlashPos);
                CurrentCursor = 0;
            }
        }
        else if (Input == 13)
        {
            if (CurrentCursor >= Directories.size())
                continue;
            CurrentDir += "/" + Directories[CurrentCursor].Name;
            CurrentCursor = 0;
        }
        else if (Input == 27)
            break;
        else if (Input == 32)
        {
            if (CurrentCursor < Directories.size())
                continue;
            size_t FileIndex = CurrentCursor - Directories.size();
            std::vector<FILE_INFO> Files = GetFilesInDirectory(CurrentDir);
            if (FindInVector(FilesToRename, Files[FileIndex]) == std::string::npos)
                FilesToRename.push_back(Files[FileIndex]);
            else
                FilesToRename.erase(FilesToRename.begin() + FindInVector(FilesToRename, Files[FileIndex]));
        }
        else if (Input == 'A' || Input == 'a')
        {
            std::vector<FILE_INFO> Files = GetFilesInDirectory(CurrentDir);
            for (const auto &File : Files)
                if (FindInVector(FilesToRename, File) == std::string::npos)
                    FilesToRename.push_back(File);
        }
        else if (Input == 'D' || Input == 'd')
        {
            std::vector<FILE_INFO> Files = GetFilesInDirectory(CurrentDir);
            for (const auto &File : Files)
                if (FindInVector(FilesToRename, File) != std::string::npos)
                    FilesToRename.erase(FilesToRename.begin() + FindInVector(FilesToRename, File));
        }
        else if (Input == 'J' || Input == 'j')
        {
            std::string Path;
            std::cout << "Please input the folder: ";
            std::getline(std::cin, Path);
            Path = StringReplaceAll(Path, "\\", "/");
            Path = std::filesystem::path(Path).lexically_normal().string();
            if (!std::filesystem::exists(Path))
            {
                printf("\033[31mError: Folder doesn't exists! \033[0mPress any key to continue...");
                _getch();
            }
            else
                CurrentDir = Path;
        }
    }
}
void SortFiles()
{
    size_t CurrentCursor = 0;
    bool SortMode = false;

    printf("\033[H\033[J\033[33mSort files: \033[0m\n");
    for (const auto &File : FilesToRename)
    {
        if (FindInVector(FilesToRename, File) == CurrentCursor)
            printf("\033[7m");
        printf("%s\033[0m\n", File.FullPath.c_str());
    }
    printf("\033[33mPress Space to toggle moving. Press Up and Down to move cursor or sorting. Press Enter to confirm.\033[0m\n\n");

    while (true)
    {
        int Input = _getch();
        if (Input == 0 || Input == 224)
        {
            Input = _getch();
            if (Input == 72 && CurrentCursor > 0)
            {
                if (SortMode)
                    std::swap(FilesToRename[CurrentCursor], FilesToRename[CurrentCursor - 1]);
                CurrentCursor--;
            }
            else if (Input == 80 && CurrentCursor < FilesToRename.size() - 1)
            {
                if (SortMode)
                    std::swap(FilesToRename[CurrentCursor], FilesToRename[CurrentCursor + 1]);
                CurrentCursor++;
            }
        }
        else if (Input == 13)
            break;
        else if (Input == 32)
            SortMode = !SortMode;
        if (CurrentCursor > 0)
            std::cout << "\033[" << CurrentCursor + 1 << ";1H\033[K" << FilesToRename[CurrentCursor - 1].FullPath << "\033[0m\n";
        std::cout << "\033[" << CurrentCursor + 2 << ";1H\033[K\033[7m" << (SortMode ? "\033[32m" : "") << FilesToRename[CurrentCursor].FullPath << "\033[0m\n";
        if (CurrentCursor < FilesToRename.size() - 1)
            std::cout << "\033[" << CurrentCursor + 3 << ";1H\033[K" << FilesToRename[CurrentCursor + 1].FullPath << "\033[0m\n";
    }
}
std::string ProceedRename(const FILE_INFO &File)
{
    size_t IndexCount = 0;
    std::string NewName = Pattern;
    size_t Position = 0;
    while (Position < NewName.length())
    {
        if (NewName[Position] == '[')
        {
            size_t EndPos = NewName.find(']', Position);
            if (EndPos == std::string::npos)
                break;
            std::string Command = NewName.substr(Position + 1, EndPos - Position - 1);
            std::string Value;
            try
            {
                if (Command == "F")
                    Value = File.FullName;
                else if (Command == ".")
                    Value = File.Extension == "" ? "" : ".";
                else if (Command == "E")
                    Value = File.Extension;
                else if (Command == "N")
                    Value = File.Name;
                else if (Command == "D")
                    Value = File.Directory;
                else if (Command == "S")
                    Value = std::to_string(File.Size);
                else if (Command == "T" || Command == "C" || Command == "W" || Command == "A")
                {
                    SYSTEMTIME SystemTime;
                    if (Command == "T")
                        GetLocalTime(&SystemTime);
                    else if (Command == "C")
                        SystemTime = File.CreationTime;
                    else if (Command == "W")
                        SystemTime = File.LastWriteTime;
                    else if (Command == "A")
                        SystemTime = File.LastAccessTime;
                    char Buffer[256];
                    GetDateFormat(LOCALE_USER_DEFAULT, 0, &SystemTime, "yyyy-MM-dd", Buffer, 256);
                    Value = Buffer;
                    GetTimeFormat(LOCALE_USER_DEFAULT, 0, &SystemTime, "HH:mm:ss", Buffer, 256);
                    Value += " ";
                    Value += Buffer;
                }
                else if (Command == "[")
                    Value = "[";
                else if (Command == "" && NewName[EndPos + 1] == ']')
                    Value = "";
                else if (Command == "U")
                {
                    UUID Uuid;
                    UuidCreate(&Uuid);
                    RPC_CSTR StringUuid;
                    UuidToString(&Uuid, &StringUuid);
                    Value = reinterpret_cast<char *>(StringUuid);
                    RpcStringFree(&StringUuid);
                }
                else if (Command == "I")
                {
                    if (IndexCount < Indexes.size())
                    {
                        Indexes[IndexCount].first += Indexes[IndexCount].second.first;
                        std::stringstream ss;
                        ss << std::fixed << std::setw(Indexes[IndexCount].second.second) << std::setfill('0') << Indexes[IndexCount].first;
                        Value = ss.str();
                    }
                    else
                    {
                        Indexes.push_back({1, {1, 1}});
                        Value = "1";
                    }
                    IndexCount++;
                }
                else
                {
                    size_t ColonPos = Command.find(':');
                    if (ColonPos != std::string::npos)
                    {
                        std::string Addison = Command.substr(ColonPos + 1);
                        Command = Command.substr(0, ColonPos);
                        if (Command == "N" || Command == "F" || Command == "E" || Command == "D")
                        {
                            if (Command == "N")
                                Value = File.Name;
                            else if (Command == "F")
                                Value = File.FullName;
                            else if (Command == "E")
                                Value = File.Extension;
                            else if (Command == "D")
                                Value = File.Directory;
                            int a = std::stoi(Addison) - 1;
                            int b = std::stoi(Addison.substr(Addison.find('~') + 1)) - 1;
                            if (a < 0)
                                a = (int)Value.length() + a + 1;
                            else if (a == 0)
                                a = 1;
                            else if (a > (int)Value.length())
                                a = (int)Value.length();
                            if (b < 0)
                                b = (int)Value.length() + b + 1;
                            else if (b == 0)
                                b = 1;
                            else if (b > (int)Value.length())
                                b = (int)Value.length();
                            if (a > b)
                                std::swap(a, b);
                            Value = Value.substr(a, b - a + 1);
                        }
                        else if (Command == "T" || Command == "C" || Command == "W" || Command == "A")
                        {
                            SYSTEMTIME SystemTime;
                            if (Command == "T")
                                GetLocalTime(&SystemTime);
                            else if (Command == "C")
                                SystemTime = File.CreationTime;
                            else if (Command == "W")
                                SystemTime = File.LastWriteTime;
                            else if (Command == "A")
                                SystemTime = File.LastAccessTime;
                            char Buffer[256];
                            GetDateFormat(LOCALE_USER_DEFAULT, 0, &SystemTime, Addison.c_str(), Buffer, 256);
                            Value = Buffer;
                        }
                        else if (Command == "I")
                        {
                            int Start = std::stoi(Addison.substr(0, Addison.find(',')));
                            int Step = std::stoi(Addison.substr(Addison.find(',') + 1, Addison.find_last_of(',') - Addison.find(',') - 1));
                            int Digits = std::stoi(Addison.substr(Addison.find_last_of(',') + 1));
                            if (IndexCount < Indexes.size())
                            {
                                Indexes[IndexCount].first += Indexes[IndexCount].second.first;
                                std::stringstream ss;
                                ss << std::fixed << std::setw(Indexes[IndexCount].second.second) << std::setfill('0') << Indexes[IndexCount].first;
                                Value = ss.str();
                            }
                            else
                            {
                                Indexes.push_back({Start, {Step, Digits}});
                                std::stringstream ss;
                                ss << std::fixed << std::setw(Digits) << std::setfill('0') << Start;
                                Value = ss.str();
                            }
                            IndexCount++;
                        }
                        else
                            Value = "[" + Command + ":" + Addison + "]";
                    }
                    else
                        Value = "[" + Command + "]";
                }
            }
            catch (...)
            {
                Value = "[" + Command + "]";
            }
            NewName.replace(Position, EndPos - Position + 1, Value);
            Position += Value.length();
        }
        else
            Position++;
    }
    if (NewName.length() > 32)
        NewName = "Name too long";
    return File.Directory + "/" + NewName;
}
void Rename()
{
    while (true)
    {
        Indexes.clear();
        printf("\033[H\033[JPattern: %s\n\n", Pattern.c_str());
        for (const auto &File : FilesToRename)
            printf("\033[31m%s\033[0m -> \033[32m%s\033[0m\n", File.FullPath.c_str(), ProceedRename(File).c_str());
        printf("\n\033[33mRandom items may have different values than the displayed ones.\nPress Enter to confirm and ESC to cancel.\033[0m\n\n");
        int Input = _getch();
        if (Input == 0 || Input == 224)
            _getch();
        else if (Input == 13)
        {
            Indexes.clear();
            for (const auto &File : FilesToRename)
            {
                std::string OldName = File.FullPath.c_str();
                std::string NewName = ProceedRename(File);
                if (!MoveFile(OldName.c_str(), NewName.c_str()))
                    printf("\033[31mError: Can not rename file \"%s\" to \"%s\" (%ld)!\033[0m\n", OldName.c_str(), NewName.c_str(), GetLastError());
                else
                    printf("\033[32mRenamed file \"%s\" to \"%s\"\033[0m.\n", OldName.c_str(), NewName.c_str());
            }
            break;
        }
        else if (Input == 8 && Pattern.length() > 0)
            Pattern = Pattern.substr(0, Pattern.length() - 1);
        else if (Input == 27)
            break;
        else if (Input >= 32 && Input <= 126)
            Pattern += Input;
    }
}
int main()
{
    InitialConsole();
    SelectFiles();
    if (FilesToRename.size() == 0)
    {
        std::cout << "Nothing to rename" << std::endl;
        return 0;
    }
    SortFiles();
    Rename();
    return 0;
}
