#include <stdio.h>
#include "apr_hash.h"
#include "ap_config.h"
#include "ap_provider.h"
#include "httpd.h"
#include "http_core.h"
#include "http_config.h"
#include "http_log.h"
#include "http_protocol.h"
#include "http_request.h"

//TO-DO: add libi2pd maybe;

typedef enum{
	false, true
} bool;

typedef struct{
	char clear[256][256];
	char i2p[256][256];
} website;

typedef struct
{
    char    context[1024];
    char    path[256];

    website addresses;
    bool     enabled;
} p2p_config;

/*$1
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Prototypes
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

static unsigned int website_count=0;

static int    p2p_handler(request_rec *r);

const char    *p2p_set_enabled(cmd_parms *cmd, void *cfg, const char *arg);
const char    *p2p_set_path(cmd_parms *cmd, void *cfg, const char *arg);
const char    *p2p_set_add_website(cmd_parms *cmd, void *cfg, const char *arg1, const char *arg2);


void          *create_dir_conf(apr_pool_t *pool, char *context);
void          *merge_dir_conf(apr_pool_t *pool, void *BASE, void *ADD);
static void   register_hooks(apr_pool_t *pool);

/*$1
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Configuration directives
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

static const command_rec    directives[] =
{
    AP_INIT_TAKE1("p2pEnabled", p2p_set_enabled, NULL, ACCESS_CONF, "Enable or disable p2p_module"),
    AP_INIT_TAKE1("p2pPath", p2p_set_path, NULL, ACCESS_CONF, "The path to mod_p2p"),
    AP_INIT_TAKE2("p2pSite", p2p_set_add_website, NULL, ACCESS_CONF, "The i2p addr website; clearhost i2phost"),
    { NULL }
};

/*$1
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Our name tag
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

module AP_MODULE_DECLARE_DATA p2p_module =
{
    STANDARD20_MODULE_STUFF,
    create_dir_conf,    /* Per-directory configuration handler */
    merge_dir_conf,     /* Merge handler for per-directory configurations */
    NULL,               /* Per-server configuration handler */
    NULL,               /* Merge handler for per-server configurations */
    directives,         /* Any directives we may have for httpd */
    register_hooks      /* Our hook registering function */
};

/*
 =======================================================================================================================
    Hook registration function
 =======================================================================================================================
 */
static void register_hooks(apr_pool_t *pool)
{
    ap_hook_handler(p2p_handler, NULL, NULL, APR_HOOK_LAST);
}

/*
 =======================================================================================================================
    Our example web service handler
 =======================================================================================================================
 */
static int p2p_handler(request_rec *r)
{
    if( strstr(r->filename, "getP2P") == 0 ) return(DECLINED);

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    p2p_config    *config = (p2p_config *) ap_get_module_config(r->per_dir_config, &p2p_module);
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    ap_set_content_type(r, "text/plain");
  //  ap_rprintf(r, "Enabled: %u\n", config->enabled);
    //ap_rprintf(r, "p2p_ADDR: %s\n", config->test);
   // ap_rprintf(r, "About: %s\n", config->context);
   // ap_rprintf(r, "handler: %s\n", r->handler);
    //ap_rprintf(r, "path: %s\n", config->path);
   // ap_rprintf(r, "filename: %s\n", r->filename);
    //ap_rprintf(r, "count_p2p addr: %d\n", website_count);
    for(unsigned int i = website_count-1;i--;){
	ap_rprintf( r, "%s = %s\n", config->addresses.i2p[i], config->addresses.clear[i] );
    }
    return OK;
}

/*
 =======================================================================================================================
    Handler for the "exampleEnabled" directive
 =======================================================================================================================
 */
const char *p2p_set_enabled(cmd_parms *cmd, void *cfg, const char *arg)
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    p2p_config    *conf = (p2p_config *) cfg;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if(conf)
    {
        if(!strcasecmp(arg, "on") || !strcasecmp(arg, "enabled")  || !strcasecmp(arg, "true"))
            conf->enabled = 1;
        else
            conf->enabled = 0;
    }

    return NULL;
}

/*
 =======================================================================================================================
    Handler for the "examplePath" directive
 =======================================================================================================================
 */
const char *p2p_set_path(cmd_parms *cmd, void *cfg, const char *arg)
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    p2p_config    *conf = (p2p_config *) cfg;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if(conf)
    {
        strcpy(conf->path, arg);
    }

    return NULL;
}

/*
 =======================================================================================================================
    Handler for the "exampleAction" directive ;
    Let's pretend this one takes one argument (file or db), and a second (deny or allow), ;
    and we store it in a bit-wise manner.
 =======================================================================================================================
 */
const char *p2p_set_add_website(cmd_parms *cmd, void *cfg, const char *arg1, const char *arg2)
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    p2p_config    *conf = (p2p_config *) cfg;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
   // conf->addresses.clear = (char*) malloc( sizeof(char*) * 256); // undefined for apache

    if(conf)
    {
	strcpy(conf->addresses.clear[website_count], arg1 );
	strcpy(conf->addresses.i2p[website_count], arg2 );   
	
	website_count++;
    }
    return NULL;
}

/*
 =======================================================================================================================
    Function for creating new configurations for per-directory contexts
 =======================================================================================================================
 */
void *create_dir_conf(apr_pool_t *pool, char *context)
{
    context = context ? context : "disabled";

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    p2p_config    *cfg = apr_pcalloc(pool, sizeof(p2p_config));
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if(cfg)
    {
        {
            /* Set some default values */
            strcpy(cfg->context, context);
            cfg->enabled = 0;
            memset(cfg->path, 0, 256);
        }
    }

    return cfg;
}

/*
 =======================================================================================================================
    Merging function for configurations
 =======================================================================================================================
 */
void *merge_dir_conf(apr_pool_t *pool, void *BASE, void *ADD)
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    p2p_config    *base = (p2p_config *) BASE;
    p2p_config    *add = (p2p_config *) ADD;
    p2p_config    *conf = (p2p_config *) create_dir_conf(pool, "Merged configuration");
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    conf->enabled = (add->enabled == 0) ? base->enabled : add->enabled;
    //conf->typeOfAction = add->typeOfAction ? add->typeOfAction : base->typeOfAction;
    for(unsigned int i = website_count-1;i--;){
    	strcpy(conf->addresses.clear[i], add->addresses.clear[i]);
    	strcpy(conf->addresses.i2p[i], add->addresses.i2p[i]);
    }
   
    strcpy(conf->path, strlen(add->path) ? add->path : base->path);
    return conf;
}
