#include "main.h"
#include "user_model.h"
#include "utils.h"

static int
extract_user_from_path (evhtp_request_t *       req,
                        struct maytrics *       maytrics,
                        char **                 user)
{
#define METRICS_REGEX_MAX_MATCH 3
    regmatch_t          pmatch[METRICS_REGEX_MAX_MATCH];

    if (regexec (maytrics->user_regex,
                 req->uri->path->full,
                 METRICS_REGEX_MAX_MATCH + 1, pmatch, 0) == REG_NOMATCH) {
        log_error ("regexec() failed.");
        return (-1);
    }
    *user = strndup (&req->uri->path->full[pmatch[1].rm_so],
                     pmatch[1].rm_eo - pmatch[1].rm_so);
    if (*user == NULL) {
        log_error ("strndup() failed.");
        return (-1);
    }
    return (0);
}

int
user_controller_get (evhtp_request_t *       req,
                     struct maytrics *       maytrics)
{
    char *              user;
    int                 status;

    if (extract_user_from_path (req, maytrics, &user) == -1) {
        return (EVHTP_RES_SERVERR);
    }

    status = get_user (req, maytrics, user);
    free (user);

    return (status);
}

int
user_controller_put (evhtp_request_t *        req,
                     struct maytrics *        maytrics)
{
    char *              user;
    int                 status;

    if (extract_user_from_path (req, maytrics, &user) == -1) {
        return (EVHTP_RES_SERVERR);
    }

    status = update_user (req, maytrics, user);
    free (user);

    return (status);
}

void
user_controller (evhtp_request_t * req, void * _maytrics)
{
    struct maytrics *    maytrics =
        (struct maytrics *)_maytrics;

    int                  status;

    switch (req->method) {
    case htp_method_PUT:
        status = user_controller_put (req, maytrics);
        break ;

    case htp_method_HEAD:
    case htp_method_GET:
        status = user_controller_get (req, maytrics);
        break ;

    default:
        status = EVHTP_RES_METHNALLOWED;
    }

    set_metrics_comment (req, status);
    evhtp_send_reply (req, status);

    return ;
}
