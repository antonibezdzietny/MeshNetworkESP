package com.example.sensormeshnetwork;

import org.json.JSONObject;

import java.io.IOException;
import java.util.Objects;

import okhttp3.Call;
import okhttp3.MediaType;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;


public class DatabaseConnector {
    private static final String BASE_URL = "http://192.168.0.200:3456/";

    private static String connection(String subPage, JSONObject jsonObject){
        OkHttpClient client = new OkHttpClient();

        RequestBody body = RequestBody.create(jsonObject.toString(),
                MediaType.parse("application/json"));

        Request request = new Request.Builder()
                .url(BASE_URL + subPage)
                .post(body)
                .build();

        Call call = client.newCall(request);
        try {
            Response response = call.execute();
            if ( response.isSuccessful() )
                return Objects.requireNonNull(response.body()).string();

            return null;
        } catch (IOException e) {
            e.printStackTrace();
        }
        return  null;
    }

    public static String createConnection(String subPage){
        JSONObject jsonObject = new JSONObject();
        return connection(subPage, jsonObject);
    }

    public static String createConnection(String subPage, JSONObject jsonObject) {
        return connection(subPage, jsonObject);
    }
}

/*
JSONObject json = new JSONObject();
json.put("TopicName", "workingRoom/Temp");
json.put("TimeFrom", "2021-10-26 22:00:00");
json.put("TimeTo", "2021-10-27 20:00:00");
 */