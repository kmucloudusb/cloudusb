package com.example.jinheesang.bluetoothtest;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.InputType;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.util.StringTokenizer;

public class WifiActivity extends AppCompatActivity {

    private EditText mInputId;
    private EditText mInputPw;
    private TextView mCurrentSsid;
    private TextView mWifiStatus;
    private ArrayAdapter<String> mConversationArrayAdapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_wifi);

        mInputId = (EditText)findViewById(R.id.input_id);
        mInputPw = (EditText)findViewById(R.id.input_pw);
        mCurrentSsid = (TextView)findViewById(R.id.current_ssid);
        mWifiStatus = (TextView)findViewById(R.id.wifi_status);

        ListView mMessageListview = (ListView) findViewById(R.id.wifi_names_listview);

        mConversationArrayAdapter = new ArrayAdapter<>( this,
                android.R.layout.simple_list_item_1 );
        mMessageListview.setAdapter(mConversationArrayAdapter);


        final AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("password");
// Set up the input
        final EditText input = new EditText(this);
// Specify the type of input expected; this, for example, sets the input as a password, and will mask the text
        input.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
        builder.setView(input);

// Set up the buttons
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                String m_Text = input.getText().toString();
                mInputPw.setText(m_Text);
                hand_out();
            }
        });
        builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.cancel();
            }
        });

        AdapterView.OnItemClickListener ssidListener= new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                mInputId.setText(mConversationArrayAdapter.getItem(position));
                builder.show();
            }
        };
        mMessageListview.setOnItemClickListener(ssidListener);


        Intent mainIntent = getIntent();
        mCurrentSsid.setText(mainIntent.getStringExtra("ssid"));
        mWifiStatus.setText(mainIntent.getStringExtra("status"));
        listing_wifi_names(mainIntent.getStringExtra("wifi_names"));

        Button sendButton = (Button)findViewById(R.id.btn_send);
        sendButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                hand_out();
            }
        });

        Button backButton = (Button)findViewById(R.id.btn_back);
        backButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Intent intent = new Intent();
                setResult(RESULT_CANCELED, intent);
                finish();
            }
        });

    }

    private void hand_out(){
        Intent intent = new Intent();
        String ssid = mInputId.getText().toString();
        String pw = mInputPw.getText().toString();

        if (ssid.length() > 0 && pw.length() > 0) {
            intent.putExtra("ssid", ssid);
            intent.putExtra("pw", pw);
            setResult(RESULT_OK, intent);
            finish();
        } else {
            Toast.makeText(getApplication(), "Incorrect WIFI ID & PW", Toast.LENGTH_SHORT).show();
        }
    }

    private void listing_wifi_names(String wifi_names){
        StringTokenizer wifi_ssid_token = new StringTokenizer(wifi_names,"@");

        while(wifi_ssid_token.hasMoreTokens()) {
            String wifi_ssid = wifi_ssid_token.nextToken();
            //wifi_ssid = wifi_ssid.substring(7, wifi_ssid.length()-1);
            mConversationArrayAdapter.insert(wifi_ssid, 0);
        }
    }
}
