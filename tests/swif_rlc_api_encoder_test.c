#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include "../src/swif_rlc_api.h"

#define VERBOSITY	0
#define SYMBOL_SIZE	1024

swif_codepoint_t codepoint = SWIF_CODEPOINT_RLC_GF_256_FULL_DENSITY_CODEC;
uint32_t	max_coding_window_size = 10;

swif_encoder_rlc_cb_t *get_test_encoder() {
    swif_encoder_rlc_cb_t *enc = (swif_encoder_rlc_cb_t *) swif_rlc_encoder_create (codepoint, VERBOSITY, SYMBOL_SIZE, max_coding_window_size);
    CU_ASSERT_PTR_NOT_NULL_FATAL(enc)
    return enc;
}

swif_status_t release_test_encoder(swif_encoder_rlc_cb_t *enc) {
    return swif_rlc_encoder_release((swif_encoder_t *)enc);
}

void generate_coefs(uint8_t *coefs, size_t size) {
    uint8_t a = 0, b = 1;
    for (int i = 0  ; i < size ; i++) {
        coefs[i] = a + b;
        a = b;
        b = coefs[i];
    }
}

void test_swif_rlc_encoder(void){

	swif_encoder_t *generic_enc = swif_rlc_encoder_create (codepoint, VERBOSITY, SYMBOL_SIZE, max_coding_window_size);
	swif_encoder_rlc_cb_t *enc = (swif_encoder_rlc_cb_t *) generic_enc;	
    CU_ASSERT_PTR_NOT_NULL_FATAL(enc)
    CU_ASSERT_EQUAL(enc->ew_left, 0)
	CU_ASSERT_EQUAL(enc->ew_right, enc->ew_left)
	CU_ASSERT_EQUAL(enc->ew_esi_right, INVALID_ESI)
	CU_ASSERT_EQUAL(enc->ew_ss_nb , 0)

	void* new_src_symbol_buf = "A";
	swif_status_t add_source_symbol_status = swif_rlc_encoder_add_source_symbol_to_coding_window( generic_enc, new_src_symbol_buf,1);

    CU_ASSERT_EQUAL(add_source_symbol_status , SWIF_STATUS_OK)
    CU_ASSERT_PTR_NOT_NULL(enc->cc_tab)
	CU_ASSERT_PTR_NOT_NULL(enc->ew_tab)
	CU_ASSERT_EQUAL(enc->ew_right , 0)
	CU_ASSERT_EQUAL(enc->ew_ss_nb , 1)
	CU_ASSERT_EQUAL(enc->ew_tab[enc->ew_right] , new_src_symbol_buf)
	CU_ASSERT_EQUAL(enc->ew_esi_right , 1)
/*
	void* new_buf = NULL;
   
	swif_status_t build_repair_symbol_status = swif_rlc_build_repair_symbol(generic_enc, new_buf);

    CU_ASSERT_PTR_NOT_NULL(new_buf);
	CU_ASSERT_EQUAL(build_repair_symbol_status , SWIF_STATUS_OK);
*/
	swif_status_t encoder_release_status = swif_encoder_release(generic_enc);
   
	CU_ASSERT_EQUAL(encoder_release_status , SWIF_STATUS_OK)

}

void test_swif_rlc_encoder_set_coding_coefs_tab_full_window() {
    swif_encoder_rlc_cb_t *enc = get_test_encoder();
    uint8_t coefs[max_coding_window_size];
    generate_coefs(coefs, max_coding_window_size);
    CU_ASSERT_EQUAL(swif_encoder_set_coding_coefs_tab((swif_encoder_t *) enc, coefs, max_coding_window_size), SWIF_STATUS_OK)
    for (int i = 0  ; i < max_coding_window_size ; i++)
        CU_ASSERT_EQUAL(enc->cc_tab[i], coefs[i])
    release_test_encoder(enc);
}

void test_swif_rlc_encoder_set_coding_coefs_tab_empty_array() {
    swif_encoder_rlc_cb_t *enc = get_test_encoder();
    uint8_t old_coefs[max_coding_window_size];
    uint8_t coefs[max_coding_window_size];
    for (int i = 0  ; i < max_coding_window_size ; i++) {
        old_coefs[i] = enc->cc_tab[i];
    }
    generate_coefs(coefs, max_coding_window_size);
    CU_ASSERT_EQUAL(swif_encoder_set_coding_coefs_tab((swif_encoder_t *) enc, coefs, 0), SWIF_STATUS_OK)
    // verify that it didn't change anything
    for (int i = 0  ; i < max_coding_window_size ; i++)
        CU_ASSERT_EQUAL(enc->cc_tab[i], old_coefs[i])
    release_test_encoder(enc);
}

void test_swif_rlc_encoder_set_coding_coefs_tab_partial_array() {
    swif_encoder_rlc_cb_t *enc = get_test_encoder();
    uint8_t old_coefs[max_coding_window_size];
    uint8_t coefs[max_coding_window_size];
    for (int i = 0  ; i < max_coding_window_size ; i++) {
        old_coefs[i] = enc->cc_tab[i];
    }
    generate_coefs(coefs, max_coding_window_size);
    CU_ASSERT_EQUAL(swif_encoder_set_coding_coefs_tab((swif_encoder_t *) enc, coefs, max_coding_window_size-1), SWIF_STATUS_OK)
    // verify that all but the last symbol have been changed
    for (int i = 0  ; i < max_coding_window_size-1 ; i++)
        CU_ASSERT_EQUAL(enc->cc_tab[i], coefs[i])
    CU_ASSERT_EQUAL(enc->cc_tab[max_coding_window_size-1], old_coefs[max_coding_window_size-1])
    release_test_encoder(enc);
}


/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{

    CU_pSuite pSuite = NULL;

    CU_basic_set_mode(CU_BRM_VERBOSE);
    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    pSuite = CU_add_suite("Suite_1", NULL, NULL);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add the tests to the suite */
    /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
    if ((NULL == CU_add_test(pSuite, "test of swif_rlc_encoder_create ()", test_swif_rlc_encoder)) ||
        (NULL == CU_add_test(pSuite, "test of swif_rlc_encoder_set_coding_coefs_tab () with a an array of size max", test_swif_rlc_encoder_set_coding_coefs_tab_full_window)) ||
        (NULL == CU_add_test(pSuite, "test of swif_rlc_encoder_set_coding_coefs_tab () with a an array of size 0", test_swif_rlc_encoder_set_coding_coefs_tab_empty_array)) ||
        (NULL == CU_add_test(pSuite, "test of swif_rlc_encoder_set_coding_coefs_tab () with a an array of size max-1", test_swif_rlc_encoder_set_coding_coefs_tab_partial_array)))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
