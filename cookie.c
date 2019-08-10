#include <napi_api.h>

#include <ldap.h>

static const char cookie_name[] = "Cookie";

// NOTE: really not sure about this, it doesn't look like init is called
// NOTE: mutliple times so i think it's safe.
napi_ref cookie_cons_ref;

static void
cookie_finalise (napi_env env, void *data, void *hint)
{
  if (data == NULL) return;
  if (*data == NULL)
    {
      free (data);
      return;
    }
  ber_bvfree ((struct berval *) *data);
  free (data);
}

static napi_value
cookie_constructor (napi_env env, napi_callback_info info)
{
  napi_status status;
  napi_value this, cookie_cons;
  size_t argc = 1;
  napi_value argv[argc];
  bool is_instance;
  struct berval **cookie = NULL;
  napi_valuetype valuetype;

  status = napi_get_cb_info (env, info, &argc, argv, &this, NULL);
  assert (status == napi_ok);

  status = napi_get_reference_value (env, cookie_cons_ref, &cookie_cons);
  assert (status == napi_ok);

  status = napi_instanceof (env, this, cookie_cons, &is_instance);
  assert (status == napi_ok);

  if (!is_instance)
    {
      napi_throw_error (env, NULL,
			"This is supposed to be a class and as such "
			"you need to declare it as a new instance.");
      return NULL;
    }

  if (argc > 1)
    {
      napi_throw_error (env, NULL, "This class requires 1 or 0 arguments");
      return NULL;
    }

  if (argc == 1)
    {
      status = napi_typeof (env, argv[0], &valuetype);
      assert (status == napi_ok);
      if (valuetype != napi_string)
	{
	  napi_throw_error (env, NULL, "Argument has to be of type string");
	  return NULL;
	}
      cookie = malloc (sizeof (void *));
      *cookie = malloc (sizeof (struct berval));
      status = napi_get_value_string_utf8 (env, argv[0], NULL,
					   0, &cookie->bv_len);
      assert (status == napi_ok);
      cookie->bv_val = malloc (cookie->bv_len + 1);
      status = napi_get_value_string_utf8 (env, argv[0], cookie->bv_val,
					   cookie->bv_len + 1, &cookie->bv_len);
      assert (status == napi_ok);
    }

  status = napi_wrap (env, this, cookie, cookie_finalise, NULL, NULL);
  assert (status == napi_ok);

  return NULL;
}

static napi_value
to_string (napi_env env, napi_callback_info info)
{
  size_t argc = 0;
  napi_value *argv, this, ret;
  napi_status status;
  struct berval *cookie;

  status = napi_get_cb_info (env, info, &argc, argv, &this, NULL);
  assert (status == napi_ok);

  status = napi_unwrap (env, this, &cookie);
  assert (status == napi_ok);

  if (cookie == NULL) return NULL;

  status = napi_create_string_utf8 (env, cookie->bv_val, cookie->bv_len, &ret);
  assert (status == napi_ok);
  return ret;
}

void
cookie_init (napi_env env, napi_value exports)
{
  napi_value cookie_cons;
  napi_status status;
  napi_properties_descriptor properties[] =
    {
     DECLARE_NAPI_METHOD ("toString", to_string)
    };

  status = napi_define_class (env, cookie_name, NAPI_AUTO_LENGTH,
			      cookie_constructor, NULL, 0,
			      popreties, &cookie_cons);
  assert (status == napi_ok);

  status = napi_create_reference (env, cookie_cons, 1, &cookie_cons_ref);
  assert (status == napi_ok);

  status = napi_set_named_property (env, exports, cookie_name, cookie_cons);
  assert (status == napi_ok);
}
