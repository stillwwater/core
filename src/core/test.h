#ifndef CORE_TEST_H_
#define CORE_TEST_H_

#include <stdio.h>
#include <string.h>

#define TEST_CAT_(A, B) A ## _ ## B ## _
#define TEST_CAT(A, B) TEST_CAT_(A, B)

struct Test {
    const char *name;
    void (*run)(Test *context__);
    Test *next;
    bool fail;
};

extern Test *test_list;

#define test(name)                                                           \
    static void TEST_CAT(test, __LINE__)(Test *context__);                   \
    static Test TEST_CAT(test_d, __LINE__)                                   \
        = {name, TEST_CAT(test, __LINE__), test_list, false};                \
    static Test *TEST_CAT(test_p, __LINE__)                                  \
        = test_list = &TEST_CAT(test_d, __LINE__);                           \
    static void TEST_CAT(test, __LINE__)([[maybe_unused]] Test *context__)

#define expect(expr)                                                         \
    do {                                                                     \
        if (!(expr)) {                                                       \
            printf("FAIL \"%s\"\n    %s:%d: %s\n",                           \
                    context__->name, __FILE__, __LINE__, #expr);             \
            context__->fail = true;                                          \
            return;                                                          \
        }                                                                    \
    } while (0)

#define test_main                                                            \
    Test *test_list;                                                         \
    int main(int argc, char *argv[])                                         \
    {                                                                        \
        int passed = 0, skipped = 0, count = 0;                              \
        char *run_test = argc > 1 ? argv[1] : NULL;                          \
        for (Test *test = test_list; test; test = test->next) {              \
            if (!run_test || !strcmp(run_test, test->name)) {                \
                test->run(test);                                             \
                if (!test->fail) {                                           \
                    printf("PASS \"%s\"\n", test->name);                     \
                    ++passed;                                                \
                }                                                            \
                ++count;                                                     \
                continue;                                                    \
            }                                                                \
            ++skipped;                                                       \
        }                                                                    \
        if (!count) {                                                        \
            if (run_test) {                                                  \
                printf("test \"%s\" not found.\n", run_test);                \
                return 2;                                                    \
            }                                                                \
            puts("no tests found.");                                         \
            return 2;                                                        \
        }                                                                    \
        if (skipped) {                                                       \
            printf("%d/%d tests passed; %d skipped.\n",                      \
                    passed, count, skipped);                                 \
            return passed != count;                                          \
        }                                                                    \
        printf("%d/%d tests passed.\n", passed, count);                      \
        return passed != count;                                              \
    }

#ifndef TEST_HAVE_MAIN
test_main
#endif // TEST_HAVE_MAIN

#endif // CORE_TEST_H_
