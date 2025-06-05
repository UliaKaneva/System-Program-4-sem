#include "task2.h"


int main(int argc, char **argv) {
    if (argc < 3) {
        return NumberArgumentsError;
    }
    int flag;
    Args *arguments = (Args *) calloc(1, sizeof(Args));
    if (!arguments) {
        return MemoryError;
    }
    Status stat = flag_handler(argc, argv, &flag, arguments);
    if (stat) {
        error_print(stat);
        free(arguments);
        return stat;
    }
    Status(*func[4])(int, char **, Args *) = {
            xorN_handler,
            mask_handler,
            copy_handler,
            find_handler
    };
    stat = func[flag](argc, argv, arguments);
    if (stat == StringNotFind){
        error_print(stat);
        return Success;
    }
    if (stat) {
        error_print(stat);
        return stat;
    }

    free(arguments);
    return Success;
}