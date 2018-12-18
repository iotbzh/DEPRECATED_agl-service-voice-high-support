
static const char _afb_description_vshlsupport[] =
    "{\"openapi\":\"3.0.0\",\"$schema\":\"http://iot.bzh/download/openapi/sch"
    "ema-3.0/default-schema.json\",\"info\":{\"description\":\"\",\"title\":\""
    "High Level Voice Service support APIs\",\"version\":\"0.1\",\"x-binding-"
    "c-generator\":{\"api\":\"vshlsupport\",\"version\":3,\"prefix\":\"afv_\""
    ",\"postfix\":\"\",\"start\":null,\"onevent\":null,\"init\":\"init\",\"sc"
    "ope\":\"\",\"private\":false,\"noconcurrency\":true}},\"servers\":[{\"ur"
    "l\":\"ws://{host}:{port}/api/monitor\",\"description\":\"TS caching bind"
    "ing\",\"variables\":{\"host\":{\"default\":\"localhost\"},\"port\":{\"de"
    "fault\":\"1234\"}},\"x-afb-events\":[{\"$ref\":\"#/components/schemas/af"
    "b-event\"}]}],\"components\":{\"schemas\":{\"afb-reply\":{\"$ref\":\"#/c"
    "omponents/schemas/afb-reply-v3\"},\"afb-event\":{\"$ref\":\"#/components"
    "/schemas/afb-event-v3\"},\"afb-reply-v3\":{\"title\":\"Generic response."
    "\",\"type\":\"object\",\"required\":[\"jtype\",\"request\"],\"properties"
    "\":{\"jtype\":{\"type\":\"string\",\"const\":\"afb-reply\"},\"request\":"
    "{\"type\":\"object\",\"required\":[\"status\"],\"properties\":{\"status\""
    ":{\"type\":\"string\"},\"info\":{\"type\":\"string\"},\"token\":{\"type\""
    ":\"string\"},\"uuid\":{\"type\":\"string\"},\"reqid\":{\"type\":\"string"
    "\"}}},\"response\":{\"type\":\"object\"}}},\"afb-event-v3\":{\"type\":\""
    "object\",\"required\":[\"jtype\",\"event\"],\"properties\":{\"jtype\":{\""
    "type\":\"string\",\"const\":\"afb-event\"},\"event\":{\"type\":\"string\""
    "},\"data\":{\"type\":\"object\"}}}},\"responses\":{\"200\":{\"descriptio"
    "n\":\"A complex object array response\",\"content\":{\"application/json\""
    ":{\"schema\":{\"$ref\":\"#/components/schemas/afb-reply\"}}}}}}}"
;


static const struct afb_verb_v3 _afb_verbs_vshlsupport[] = {
    {
        .verb = NULL,
        .callback = NULL,
        .auth = NULL,
        .info = NULL,
        .session = 0,
        .vcbdata = NULL,
        .glob = 0
	}
};

const struct afb_binding_v3 afbBindingV3 = {
    .api = "vshlsupport",
    .specification = _afb_description_vshlsupport,
    .info = "",
    .verbs = _afb_verbs_vshlsupport,
    .preinit = NULL,
    .init = init,
    .onevent = NULL,
    .userdata = NULL,
    .noconcurrency = 1
};

