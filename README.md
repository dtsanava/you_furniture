# 3D-редактор для смартфонов

Редактор полигонального моделирования для Android в духе Blender.
Техническое задание: https://claude.ai/code/artifact/b9f76d2d-aa1a-4552-b965-c6aa7f9ef05b

## Что умеет сейчас (этап «Каркас приложения»)

- Вьюпорт OpenGL ES 3.0 с сеткой пола, осями X (красная) и Y (зелёная) и кубом.
- Камера жестами: один палец вращает, два пальца сдвигают, щипок приближает.

## Структура

| Папка | Что внутри |
| --- | --- |
| `core/` | Ядро на C++17 без зависимостей от Android: математика, камера, сетка, примитивы, тесты |
| `app/src/main/cpp/` | Отрисовка на OpenGL ES 3.0 и мост JNI |
| `app/src/main/java/` | Android-приложение на Kotlin: экран и жесты |
| `.github/workflows/` | Сборка APK и тесты ядра на каждый push |

Система координат как в Blender: правая, ось Z вверх.

## Как получить APK

Каждый push в `main` запускает GitHub Actions. Готовый `app-debug.apk` лежит в артефакте
`m3d-debug-apk` на странице запуска (вкладка Actions). Его можно скачать на телефон и установить,
разрешив установку из неизвестных источников.

## Локальная сборка

Тесты ядра (нужны CMake и компилятор C++):

```sh
cmake -S core -B build/core
cmake --build build/core
ctest --test-dir build/core --output-on-failure
```

APK (нужны Android SDK и NDK, проще всего через Android Studio):

```sh
./gradlew assembleDebug
```
