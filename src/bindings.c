#include <string.h>
#include <stdio.h>

#include <caml/alloc.h>
#include <caml/bigarray.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>

#include <choices.h>

CAMLprim value fzy_search_for_item(value vHaystack, value vNeedle) {
    CAMLparam2(vHaystack, vNeedle);
    CAMLlocal3(match_item, matched_chars, return_array);

    options_t options;
    choices_t choices;
    options_init(&options);
    choices_init(&choices, &options);

    const int nItems = Wosize_val(vHaystack);

    for (int i = 0; i < nItems; ++i) {
        choices_add(&choices, String_val(Field(vHaystack, i)));
    }

    const char *needle = String_val(vNeedle);
    const int needleSize = strlen(needle);
    choices_search(&choices, needle);

    return_array = caml_alloc(choices_available(&choices), 0);

    // Fzy only stores scores for those with an actual match.
    for (int i = 0; i < choices_available(&choices); ++i) {

        // Get the search terms that actually matched.
        const char *matchTerm = choices_get(&choices, i);
        
        // Now go back and get the score again, and populate the
        // match locations.
        const int n = strlen(matchTerm) + 1;
        size_t positions[n];
        for (int i = 0; i < n; i++)
          positions[i] = -1;

        const double score = match_positions(needle, matchTerm, &positions[0]);

        match_item = caml_alloc(3, 0);

        matched_chars = caml_alloc(needleSize, 0);

        for (int i = 0; i < needleSize; ++i)
            Store_field(matched_chars, i, Val_int(positions[i]));

        Store_field(match_item, 0, caml_copy_string(matchTerm));
        Store_field(match_item, 1, caml_copy_double(score));
        Store_field(match_item, 2, matched_chars);

        Store_field(return_array, i, match_item);
    }

    choices_destroy(&choices);

    CAMLreturn(return_array);
}
