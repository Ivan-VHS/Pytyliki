#include "syntax_pie.h"

#include <string.h>

/* Function verb, room metaphor and syntax abstraction form one stable layer. */
static const pl_syntax_pie_layer layers[PL_SYNTAX_PIE_LAYER_COUNT] = {
    { 1u,  "Зал",             "Комната",     PL_SYNTAX_LEXEME,             "Лексема",                    "Создать минимальную именованную функциональную единицу." },
    { 2u,  "Заливать",        "Помещение",   PL_SYNTAX_TOKEN,              "Токен",                      "Распознать и зарегистрировать входной токен." },
    { 3u,  "Заложить",        "Покои",       PL_SYNTAX_WORD_FORM,          "Словоформа",                 "Связать токен с нормализованной словоформой." },
    { 4u,  "Залатать",        "Палата",      PL_SYNTAX_COLLOCATION,        "Словосочетание",             "Собрать совместимые словоформы в устойчивую группу." },
    { 5u,  "Залечить",        "Зала",        PL_SYNTAX_SYNTAGMA,           "Синтагма",                   "Проверить внутреннюю связь и границы синтагмы." },
    { 6u,  "Заладить",        "Зал",         PL_SYNTAX_PHRASE,             "Фраза",                      "Сформировать и проверить законченную фразу." },
    { 7u,  "Залучить",        "Гостиная",    PL_SYNTAX_TURNOVER,           "Оборот",                     "Включить оборот в окружающий синтаксический контекст." },
    { 8u,  "Залучать",        "Прихожая",    PL_SYNTAX_CLAUSE,             "Клауза",                     "Построить клаузу и связать её аргументы." },
    { 9u,  "Залповать",       "Холл",        PL_SYNTAX_PREDICATIVE,        "Предикативная конструкция",  "Проверить предикацию, субъект и предикат." },
    { 10u, "Залетать",        "Фойе",        PL_SYNTAX_SIMPLE_SENTENCE,    "Простое предложение",        "Собрать одну предикативную основу." },
    { 11u, "Залететь",        "Лаунж",       PL_SYNTAX_COMPLEX_SENTENCE,   "Сложное предложение",        "Связать предикативные части по правилам грамматики." },
    { 12u, "Заливаться",      "Салон",       PL_SYNTAX_BLOCK,              "Синтаксический блок",        "Скомпоновать предложения в логически цельный блок." },
    { 13u, "Заложенный",      "Кабинет",     PL_SYNTAX_NESTED,             "Вложенная конструкция",      "Разобрать вложенную область с отдельным контекстом." },
    { 14u, "Заложить",        "Спальня",     PL_SYNTAX_PARSE_TREE,         "Дерево разбора",             "Построить дерево синтаксического разбора." },
    { 15u, "Заложиться",      "Опочивальня", PL_SYNTAX_AST,                "Абстрактное синтаксическое дерево", "Построить AST без несущественных синтаксических деталей." },
    { 16u, "Залавливать",     "Чертог",      PL_SYNTAX_GRAMMAR_RULE,       "Грамматическое правило",     "Применить правило грамматики к узлам AST." },
    { 17u, "Залавливающий",   "Покой",       PL_SYNTAX_CFG,                "Контекстно-свободная грамматика", "Проверить выводимость по контекстно-свободной грамматике." },
    { 18u, "Залавливающийся", "Павильон",    PL_SYNTAX_GRAPH,              "Синтаксический граф",        "Преобразовать связи разбора в ориентированный граф." },
    { 19u, "Залегендировать", "Апартаменты", PL_SYNTAX_COMPOSITION,       "Композиционная структура",  "Скомпоновать уровни с сохранением зависимостей." },
    { 20u, "Залегендарить",   "Анфилада",    PL_SYNTAX_HIERARCHY,         "Иерархическая грамматика",  "Проверить иерархию грамматик и вложенных правил." }
};

static int equal_ascii_fold(const char *a, const char *b) {
    unsigned char ca, cb;
    if (a == NULL || b == NULL) return 0;
    while (*a != '\0' && *b != '\0') {
        ca = (unsigned char)*a++;
        cb = (unsigned char)*b++;
        if (ca >= 'A' && ca <= 'Z') ca = (unsigned char)(ca + ('a' - 'A'));
        if (cb >= 'A' && cb <= 'Z') cb = (unsigned char)(cb + ('a' - 'A'));
        if (ca != cb) return 0;
    }
    return *a == '\0' && *b == '\0';
}

size_t pl_syntax_pie_catalog(const pl_syntax_pie_layer **out) {
    if (out == NULL) return 0u;
    *out = layers;
    return PL_SYNTAX_PIE_LAYER_COUNT;
}

int pl_syntax_pie_get(unsigned id, const pl_syntax_pie_layer **out) {
    if (out == NULL || id == 0u || id > PL_SYNTAX_PIE_LAYER_COUNT) return -1;
    *out = &layers[id - 1u];
    return 0;
}

static int find_value(const char *value, const pl_syntax_pie_layer **out, unsigned field) {
    size_t i;
    if (value == NULL || value[0] == '\0' || out == NULL) return -1;
    for (i = 0u; i < PL_SYNTAX_PIE_LAYER_COUNT; ++i) {
        const char *candidate = field == 0u ? layers[i].function_name :
            (field == 1u ? layers[i].room_name : layers[i].level_name);
        if (equal_ascii_fold(value, candidate)) {
            *out = &layers[i];
            return 0;
        }
    }
    return -2;
}

int pl_syntax_pie_find_function(const char *name, const pl_syntax_pie_layer **out) {
    return find_value(name, out, 0u);
}

int pl_syntax_pie_find_room(const char *name, const pl_syntax_pie_layer **out) {
    return find_value(name, out, 1u);
}

int pl_syntax_pie_find_level(const char *name, const pl_syntax_pie_layer **out) {
    return find_value(name, out, 2u);
}

const char *pl_syntax_pie_level_name(pl_syntax_level level) {
    if ((int)level < (int)PL_SYNTAX_LEXEME || (unsigned)level >= PL_SYNTAX_PIE_LAYER_COUNT)
        return "invalid";
    return layers[(unsigned)level].level_name;
}
