package com.example.sensormeshnetwork;


import android.app.Activity;

public class SensorType {
    public enum Type {
        DEFAULT,
        TEMPERATURE,
        HUMIDITY,
        ILLUMINANCE
    }

    public static SensorType.Type getType(String s){
        switch (s.toUpperCase()){
            case "TEMPERATURE":
                return Type.TEMPERATURE;
            case "HUMIDITY":
                return Type.HUMIDITY;
            case "ILLUMINANCE":
                return Type.ILLUMINANCE;
            default:
                return Type.DEFAULT;
        }
    }

    public static int getIconId(Activity activity, SensorType.Type type){
        switch (type){
            case TEMPERATURE:
                return activity.getResources().getIdentifier("thermometer",
                        "drawable", activity.getPackageName());
            case HUMIDITY:
                return activity.getResources().getIdentifier("humidity",
                        "drawable", activity.getPackageName());
            case ILLUMINANCE:
                return activity.getResources().getIdentifier("illuminance",
                        "drawable", activity.getPackageName());
            default:
                return activity.getResources().getIdentifier("defaultt",
                        "drawable", activity.getPackageName());
        }
    }
}
