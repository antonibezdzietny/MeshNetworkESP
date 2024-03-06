package com.example.sensormeshnetwork;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.json.JSONException;
import org.json.JSONObject;

public class SensorListAdapter extends ArrayAdapter {
    Activity activity;
    JSONObject [] jsonObjects;


    public SensorListAdapter(Activity activity, JSONObject [] jsonObjects){
        super(activity, R.layout.sensor_item, jsonObjects);
        this.activity = activity;
        this.jsonObjects = jsonObjects;
    }


    @SuppressLint("ResourceAsColor")
    @NonNull
    @Override
    public View getView(int position, @Nullable View convertView, @NonNull ViewGroup parent) {
        View rowView;
        if( convertView == null ){
            LayoutInflater inflater = activity.getLayoutInflater();
            rowView = inflater.inflate(R.layout.sensor_item, parent, false);
        }else{
            rowView = convertView;
        }

        LinearLayout linearLayout = (LinearLayout) rowView.findViewById(R.id.row_layout);
        ImageView imageView = (ImageView) rowView.findViewById(R.id.sensorImageView);
        TextView mainText = (TextView) rowView.findViewById(R.id.sensorMainText);
        TextView descriptionText = (TextView) rowView.findViewById(R.id.sensorDescriptionText);


        try {
            imageView.setImageResource(SensorType.getIconId(activity, SensorType
                    .getType(jsonObjects[position].getString("SensorType"))));
            mainText.setText(jsonObjects[position].getString("ProperName"));
            descriptionText.setText(jsonObjects[position].getString("TopicName"));
        } catch (JSONException e) {
            e.printStackTrace();
        }

        return rowView;
    }
}
