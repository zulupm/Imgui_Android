package me.sfalexrog.imguidemo;

import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;
import okhttp3.WebSocket;
import okhttp3.WebSocketListener;
import okhttp3.mockwebserver.MockResponse;
import okhttp3.mockwebserver.MockWebServer;
import org.junit.Test;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class WebSocketConnectivityTest {
    @Test
    public void webSocketReceivesMessage() throws Exception {
        MockWebServer server = new MockWebServer();
        CountDownLatch latch = new CountDownLatch(1);

        server.enqueue(new MockResponse().withWebSocketUpgrade(new WebSocketListener() {
            @Override
            public void onOpen(WebSocket webSocket, Response response) {
                webSocket.send("hello");
            }
        }));
        server.start();

        OkHttpClient client = new OkHttpClient();
        Request request = new Request.Builder().url(server.url("/")).build();
        client.newWebSocket(request, new WebSocketListener() {
            @Override
            public void onMessage(WebSocket webSocket, String text) {
                assertEquals("hello", text);
                latch.countDown();
            }
        });

        assertTrue(latch.await(5, TimeUnit.SECONDS));
        server.shutdown();
    }
}
