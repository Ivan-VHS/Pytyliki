#include <stdio.h>
#include <string.h>

typedef struct {
    const char *id;
    const char *title;
    const char *evidence;
    const char *summary;
    const char *timestamps;
} wellbeing_topic;

static const wellbeing_topic TOPICS[] = {
    {
        "destigmatization",
        "Розмова без стигми",
        "transcript excerpt",
        "Учасниці говорять про страх називати депресію та про те, що звернення по допомогу помилково сприймають як слабкість. Це не діагностичний критерій.",
        "00:00, 00:14, 00:22, 00:28, 00:36"
    },
    {
        "personal-experiences",
        "Особистий досвід учасниць",
        "transcript excerpt",
        "Наті згадує рішення почати приймати антидепресант; Мотричка — втрату Юри; Таня — п'ятий рік прийому. Це особисті свідчення, а не універсальні рекомендації.",
        "02:06, 02:14"
    },
    {
        "reported-depressive-episode",
        "Розповідь про епізод у 2019 році",
        "transcript excerpt",
        "Одна учасниця описує гострий стан, звернення до психіатра та призначене лікування. Її історія не дозволяє встановлювати діагноз іншій людині.",
        "03:27–07:55"
    },
    {
        "support-pyramid",
        "Люди, рух, терапія",
        "transcript excerpt",
        "Учасниця переказує модель підтримки з фільму Stutz. Це згадана в розмові модель, а не клінічний протокол чи порядок лікування.",
        "01:17:27–01:18:07"
    },
    {
        "normalization-and-help-seeking",
        "Нормалізація звернення по допомогу",
        "transcript excerpt",
        "Учасниці наголошують, що депресія не є слабкістю чи лінню, та радять звертатися до фахівців. Ліки — лише за рецептом психіатра. Число 280 мільйонів — лише неперевірена актуально цифра, озвучена у відео.",
        "01:18:16–01:19:53"
    },
    {
        "antidepressant-myths",
        "Міфи про антидепресанти",
        "insufficient transcript",
        "Тема є в назві відео, але в наданих уривках немає переліку міфів чи відповідей експертки. Не вигадувати твердження про препарати, ефективність або побічні ефекти.",
        "no available timestamp"
    }
};

static void print_header(void) {
    puts("Mental Wellbeing & Depression Awareness v1");
    puts("Source: eбаут — «Депресія — тема табу? Ознаки депресії, міфи довкола антидепресантів, як допомогти собі»");
    puts("Published: 2025-11-23 | Duration: 01:20:01");
    puts("https://www.youtube.com/watch?v=brNbnipJ-i8");
    puts("Transcript coverage: 00:00–00:07:55 and 01:17:16–01:19:53.");
    puts("The middle section is unavailable in the supplied transcript; no reconstruction is attempted.");
}

static void print_topic(const wellbeing_topic *topic) {
    printf("\n%s [%s]\n%s\nTimestamps: %s\n",
           topic->title, topic->evidence, topic->summary, topic->timestamps);
}

static void print_guardrails(void) {
    puts("\nSafety boundary:");
    puts("- Educational reference only; not a medical or clinical product.");
    puts("- No diagnosis, screening, risk scoring, or symptom interpretation.");
    puts("- No mental-state inference from processor, device, audio, or work telemetry.");
    puts("- No medication or dosage instructions; consult a qualified clinician.");
    puts("- Personal stories are not universal clinical facts.");
    puts("- Missing transcript passages remain unavailable, not guessed.");
}

int main(int argc, char **argv) {
    size_t i;
    if (argc == 1 || (argc == 2 && strcmp(argv[1], "--catalog") == 0)) {
        print_header();
        for (i = 0U; i < sizeof(TOPICS) / sizeof(TOPICS[0]); ++i)
            print_topic(&TOPICS[i]);
        print_guardrails();
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "--topic") == 0) {
        print_header();
        for (i = 0U; i < sizeof(TOPICS) / sizeof(TOPICS[0]); ++i) {
            if (strcmp(argv[2], TOPICS[i].id) == 0) {
                print_topic(&TOPICS[i]);
                print_guardrails();
                return 0;
            }
        }
        fprintf(stderr, "Unknown wellbeing topic: %s\n", argv[2]);
        return 2;
    }
    fprintf(stderr, "Usage: pytyliki-wellbeing [--catalog | --topic TOPIC_ID]\n");
    return 2;
}
