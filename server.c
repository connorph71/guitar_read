#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static struct lws *client_wsi = NULL;

static int callback(struct lws *wsi,
                    enum lws_callback_reasons reason,
                    void *user, void *in, size_t len)
{
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            printf("Client connected\n");
            client_wsi = wsi;
            break;

        case LWS_CALLBACK_CLOSED:
            printf("Client disconnected\n");
            client_wsi = NULL;
            break;

        default:
            break;
    }
    return 0;
}

static struct lws_protocols protocols[] = {
    {
        "example-protocol",
        callback,
        0,
        4096,
    },
    { NULL, NULL, 0, 0 }
};

int main(void)
{
    struct lws_context_creation_info info;
    struct lws_context *context;

    memset(&info, 0, sizeof(info));
    info.port = 8766;
    info.protocols = protocols;

    context = lws_create_context(&info);
    if (!context) {
        fprintf(stderr, "Failed to create context\n");
        return -1;
    }

    printf("Starting server...\n");


	FILE *mic_pipe = popen("stdbuf -oL ./mic_read", "r");
	if (!mic_pipe) {
    	perror("popen failed");
    	return -1;
	}

    char input[256];

	while (1) {
    	lws_service(context, 100);

	    if (client_wsi && fgets(input, sizeof(input), mic_pipe)) {
    	    size_t len = strlen(input);

        	unsigned char buf[LWS_PRE + 256];
        	memcpy(&buf[LWS_PRE], input, len);

	        lws_write(client_wsi,
						&buf[LWS_PRE],
        	    	    len,
        		        LWS_WRITE_TEXT);
    	}
	}

    lws_context_destroy(context);
    return 0;
}