#include <windows.h>
#include <iostream>

// Структура моба
struct Mob {
    int id;
    float x, y, z;
    int hp;
};

// Shared Memory
HANDLE hMapFile = nullptr;
Mob* pMobData = nullptr;

// Функция чтения моба по индексу
Mob ReadMob(int index) {
    uintptr_t address = mobListBase + index * mobEntrySize;
    Mob mob;
    mob.id = *(int*)(address + 0x00);
    mob.x = *(float*)(address + 0x110);
    mob.y = *(float*)(address + 0x114);
    mob.z = *(float*)(address + 0x118);
    mob.hp = *(int*)(address + 0x100);
    return mob;
}

// Поток обновления данных
void UpdateMobs() {
    while (true) {
        for (int i = 0; i < 10; ++i) { // до 10 мобов
            Mob mob = ReadMob(i);
            if (mob.hp > 0) {
                pMobData[i] = mob;
            } else {
                pMobData[i] = {0};  // очищаем убитых мобов
            }
        }
        Sleep(1000); // Обновление каждые N секунд
    }
}

// Экспортируемая функция для CE / Injector'ов
extern "C" __declspec(dllexport) void StartBot() {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "[+] RF Warden Bot загружена" << std::endl;

    // Создание Shared Memory
    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        nullptr,
        PAGE_READWRITE,
        0,
        sizeof(Mob) * 10,
        L"Local\\RFMobsSharedMemory"
    );

    if (!hMapFile) {
        std::cerr << "[!] Не удалось создать Shared Memory!" << std::endl;
        return;
    }

    pMobData = (Mob*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Mob) * 10);
    if (!pMobData) {
        std::cerr << "[!] Не удалось получить доступ к памяти" << std::endl;
        return;
    }

    // Запуск потока бота
    CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)UpdateMobs, nullptr, 0, nullptr);
}

// Точка входа Windows
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)StartBot, hModule, 0, nullptr);
    }
    return TRUE;
}
