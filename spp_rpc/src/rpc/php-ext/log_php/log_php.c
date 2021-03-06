
/**
 * Tencent is pleased to support the open source community by making MSEC available.
 *
 * Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the GNU General Public License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. You may 
 * obtain a copy of the License at
 *
 *     https://opensource.org/licenses/GPL-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the 
 * License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions
 * and limitations under the License.
 */


/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_log_php.h"
#include "srpc_cintf.h"
#include "configini_c.h"

/* If you declare any globals in php_log_php.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(log_php)
*/

/* True global resources - no need for thread safety here */
static int le_log_php;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("log_php.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_log_php_globals, log_php_globals)
    STD_PHP_INI_ENTRY("log_php.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_log_php_globals, log_php_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_log_php_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_log_php_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "log_php", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/
PHP_FUNCTION(nglog_set_option)
{
    zval *val;
    zval *key;

    if (ZEND_NUM_ARGS() != 2) {
        return;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z/z/", &key, &val) == FAILURE) {
        return;
    }

    convert_to_string(key);
    convert_to_string(val);

    set_log_option_str(Z_STRVAL_P(key), Z_STRVAL_P(val));

    zval_dtor(key);
    zval_dtor(val);
}

PHP_FUNCTION(nglog_error)
{
    char *log_str;
    int   len;

    if (ZEND_NUM_ARGS() != 1) {
        return;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &log_str, &len) == FAILURE) {
        return;
    }

    msec_log_error(log_str);
}

PHP_FUNCTION(nglog_info)
{
    char *log_str;
    int   len;

    if (ZEND_NUM_ARGS() != 1) {
        return;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &log_str, &len) == FAILURE) {
        return;
    }

    msec_log_info(log_str);
}

PHP_FUNCTION(nglog_debug)
{
    char *log_str;
    int   len;

    if (ZEND_NUM_ARGS() != 1) {
        return;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &log_str, &len) == FAILURE) {
        return;
    }

    msec_log_debug(log_str);
}

PHP_FUNCTION(nglog_fatal)
{
    char *log_str;
    int   len;

    if (ZEND_NUM_ARGS() != 1) {
        return;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &log_str, &len) == FAILURE) {
        return;
    }

    msec_log_fatal(log_str);
}

PHP_FUNCTION(get_config)
{
    char *filename = "../etc/config.ini";
    char *session;
    char *key;
    char *val;
    int flen;
    int slen;
    int klen;

    if ((ZEND_NUM_ARGS() != 2) && (ZEND_NUM_ARGS() != 3)) {
        RETURN_NULL();
    }

    if (ZEND_NUM_ARGS() == 2) {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &session, &slen, &key, &klen) == FAILURE) {
            RETURN_NULL();
        }
    } else if (ZEND_NUM_ARGS() == 3) {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &filename, &flen, &session, &slen, &key, &klen) == FAILURE) {
            RETURN_NULL();
        }
    }

    val = GetConfig(filename, session, key);
    if (NULL == val) {
        RETURN_NULL();
    }

    RETVAL_STRING(val, 1);
    free(val);
}

/* {{{ php_log_php_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_log_php_init_globals(zend_log_php_globals *log_php_globals)
{
	log_php_globals->global_value = 0;
	log_php_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(log_php)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(log_php)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(log_php)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(log_php)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(log_php)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "log_php support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ log_php_functions[]
 *
 * Every user visible function must have an entry in log_php_functions[].
 */
const zend_function_entry log_php_functions[] = {
	PHP_FE(confirm_log_php_compiled,	NULL)		/* For testing, remove later. */
    PHP_FE(nglog_set_option,    NULL)
    PHP_FE(nglog_debug,         NULL)
    PHP_FE(nglog_fatal,         NULL)
    PHP_FE(nglog_error,         NULL)
    PHP_FE(nglog_info,          NULL)
    PHP_FE(get_config,          NULL)
	PHP_FE_END	/* Must be the last line in log_php_functions[] */
};
/* }}} */

/* {{{ log_php_module_entry
 */
zend_module_entry log_php_module_entry = {
	STANDARD_MODULE_HEADER,
	"log_php",
	log_php_functions,
	PHP_MINIT(log_php),
	PHP_MSHUTDOWN(log_php),
	PHP_RINIT(log_php),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(log_php),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(log_php),
	PHP_LOG_PHP_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_LOG_PHP
ZEND_GET_MODULE(log_php)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
