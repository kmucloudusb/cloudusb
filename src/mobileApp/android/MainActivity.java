package com.example.jinheesang.bluetoothtest;
/*
 *
 *
 * 참고
 * https://github.com/googlesamples/android-BluetoothChat
 * http://www.kotemaru.org/2013/10/30/android-bluetooth-sample.html
 */

        import java.io.IOException;
        import java.io.InputStream;
        import java.io.OutputStream;
        import java.util.Set;
        import java.util.UUID;

        import android.bluetooth.BluetoothAdapter;
        import android.bluetooth.BluetoothDevice;
        import android.bluetooth.BluetoothSocket;
        import android.content.DialogInterface;
        import android.content.Intent;
        import android.os.AsyncTask;
        import android.os.Bundle;
        import android.support.v7.app.AlertDialog;
        import android.support.v7.app.AppCompatActivity;
        import android.util.Log;
        import android.view.View;
        import android.widget.ArrayAdapter;
        import android.widget.Button;
        import android.widget.ListView;
        import android.widget.TextView;
        import android.widget.Toast;



public class MainActivity extends AppCompatActivity
{
    enum B_MESSAGE_ID {
        MESG_NONE, MESG_ERROR,
        REQU_GET_WIFI_INFO, RESP_GET_WIFI_INFO,
        REQU_SET_WIFI, RESP_SET_WIFI,
        REQU_SET_CLIENT_SECRET, RESP_SET_CLIENT_SECRET,
        REQU_GET_USERS_LIST, RESP_GET_USERS_LIST,
        REQU_ADD_DRIVE_AUTH, RESP_ADD_DRIVE_AUTH,
        REQU_CHANGE_DRIVE_AUTH, RESP_CHANGE_DRIVE_AUTH,
        REQU_DEL_DRIVE_AUTH, RESP_DEL_DRIVE_AUTH,
        REQU_AVAILABLE_WIFI_NAMES, RESP_AVAILABLE_WIFI_NAMES
    }

    enum B_RESPONSE_STATE {
        RESULT_OK,
        RESULT_FAIL,
        RESULT_ERROR
    }

    private final int REQUEST_BLUETOOTH_ENABLE = 100;
    private final int REQUEST_WIFI_SETTING = 101;

    private TextView mConnectionStatus;
    private TextView mSsidStatus;
    private TextView mWifiStatus;

    ConnectedTask mConnectedTask = null;
    static BluetoothAdapter mBluetoothAdapter;
    private String mConnectedDeviceName = null;
    private ArrayAdapter<String> mConversationArrayAdapter;
    static boolean isConnectionError = false;
    private static final String TAG = "BluetoothClient";

    // BMessage Class
    private class BMessage{
        private int id = B_MESSAGE_ID.MESG_NONE.ordinal();
        private int state = B_RESPONSE_STATE.RESULT_FAIL.ordinal();
        private String param1 = " ";
        private String param2 = " ";

        // Send Message
        BMessage(B_MESSAGE_ID id, B_RESPONSE_STATE state, String param1, String param2){
            this.id = id.ordinal();
            this.state = state.ordinal();
            this.param1 = param1;
            this.param2 = param2;
        }

        private boolean sendMsg(){
            String msgString = id + "$" + state + "$" + param1 + "$" + param2 + "$";
            sendMessage(msgString);
            return true;
        }

        // Request Message
        BMessage(String requset){
            String[] parsed = requset.split("\\$", 4);
            Log.d( TAG, parsed[0]);
            Log.d( TAG, parsed[1]);
            Log.d( TAG, parsed[2]);
            Log.d( TAG, parsed[3]);

            id = Integer.parseInt(parsed[0]);
            state = Integer.parseInt(parsed[1]);
            param1 = parsed[2];
            param2 = parsed[3];
        }

        private boolean execute(){

            switch (B_MESSAGE_ID.values()[id]){
                case RESP_SET_WIFI:
                case RESP_GET_WIFI_INFO:
                    responseGetWifiInfo();
                    break;

                case RESP_AVAILABLE_WIFI_NAMES:
                    responseAvailableWifiNames();
                    break;

                default:
                    ;
            }
            return true;
        }

        private boolean responseGetWifiInfo(){
            String ssid = param1;

            if(state == B_RESPONSE_STATE.RESULT_OK.ordinal()){
                mWifiStatus.setText("Connected");
                mSsidStatus.setText("SSID: " + ssid);
                Toast.makeText(getApplication(), "WIFI: connected", Toast.LENGTH_SHORT).show();

                return true;
            }
            else{
                mWifiStatus.setText("Fail");
                mSsidStatus.setText(" ");
                Toast.makeText(getApplication(), "WIFI: failed" , Toast.LENGTH_SHORT).show();

                return false;
            }

        }

        private boolean responseAvailableWifiNames(){
            String wifi_names = param1;
            if(state == B_RESPONSE_STATE.RESULT_OK.ordinal()){
                Intent wifi_intent = new Intent(MainActivity.this, WifiActivity.class);
                wifi_intent.putExtra("status", mWifiStatus.getText().toString());
                wifi_intent.putExtra("ssid", mSsidStatus.getText().toString());
                wifi_intent.putExtra("wifi_names", wifi_names);
                startActivityForResult(wifi_intent, REQUEST_WIFI_SETTING);

                return true;
            }
            return false;
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button wifiButton = (Button)findViewById(R.id.btn_wifi);
        wifiButton.setOnClickListener(new View.OnClickListener(){
            public void onClick(View v){
                BMessage message = new BMessage(
                        B_MESSAGE_ID.REQU_AVAILABLE_WIFI_NAMES,
                        B_RESPONSE_STATE.RESULT_OK,
                        " ", " "
                );
                message.sendMsg();

            }
        });
        mConnectionStatus = (TextView)findViewById(R.id.connection_status_textview);
        mSsidStatus = (TextView)findViewById(R.id.current_ssid);
        mWifiStatus = (TextView)findViewById(R.id.wifi_status);
        ListView mMessageListview = (ListView) findViewById(R.id.message_listview);

        mConversationArrayAdapter = new ArrayAdapter<>( this,
                android.R.layout.simple_list_item_1 );
        mMessageListview.setAdapter(mConversationArrayAdapter);


        Log.d( TAG, "Initalizing Bluetooth adapter...");
        //1.블루투스 사용 가능한지 검사합니다.
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBluetoothAdapter == null) {
            showErrorDialog("This device is not implement Bluetooth.");
            return;
        }

        if (!mBluetoothAdapter.isEnabled()) {
            Intent intent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(intent, REQUEST_BLUETOOTH_ENABLE);
        }
        else {
            Log.d(TAG, "Initialisation successful.");

            //2. 페어링 되어 있는 블루투스 장치들의 목록을 보여줍니다.
            //3. 목록에서 블루투스 장치를 선택하면 선택한 디바이스를 인자로 하여
            //   doConnect 함수가 호출됩니다.
            showPairedDevicesListDialog();

        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if ( mConnectedTask != null ) {

            mConnectedTask.cancel(true);
        }
    }

    //runs while listening for incoming connections.
    private class ConnectTask extends AsyncTask<Void, Void, Boolean> {

        private BluetoothSocket mBluetoothSocket = null;
        private BluetoothDevice mBluetoothDevice = null;

        ConnectTask(BluetoothDevice bluetoothDevice) {
            mBluetoothDevice = bluetoothDevice;
            mConnectedDeviceName = bluetoothDevice.getName();

            //SPP
            UUID uuid = UUID.fromString("00001101-0000-1000-8000-00805f9b34fb");

            // Get a BluetoothSocket for a connection with the
            // given BluetoothDevice
            try {
                mBluetoothSocket = mBluetoothDevice.createRfcommSocketToServiceRecord(uuid);
                Log.d( TAG, "create socket for "+mConnectedDeviceName);

            } catch (IOException e) {
                Log.e( TAG, "socket create failed " + e.getMessage());
            }

            mConnectionStatus.setText("connecting...");
        }


        @Override
        protected Boolean doInBackground(Void... params) {

            // Always cancel discovery because it will slow down a connection
            mBluetoothAdapter.cancelDiscovery();

            // Make a connection to the BluetoothSocket
            try {
                // This is a blocking call and will only return on a
                // successful connection or an exception
                mBluetoothSocket.connect();
            } catch (IOException e) {
                // Close the socket
                try {
                    mBluetoothSocket.close();
                } catch (IOException e2) {
                    Log.e(TAG, "unable to close() " +
                            " socket during connection failure", e2);
                }

                return false;
            }

            return true;
        }


        @Override
        protected void onPostExecute(Boolean isSucess) {

            if ( isSucess ) {
                connected(mBluetoothSocket);
            }
            else{

                isConnectionError = true;
                Log.d( TAG,  "Unable to connect device");
                showErrorDialog("Unable to connect device");
            }
        }
    }


    public void connected( BluetoothSocket socket ) {
        mConnectedTask = new ConnectedTask(socket);
        mConnectedTask.execute();
    }

    private Boolean firstMessage(){
        BMessage message = new BMessage(
                B_MESSAGE_ID.REQU_GET_WIFI_INFO,
                B_RESPONSE_STATE.RESULT_OK,
                " "," "
        );

        message.sendMsg();
        return true;
    }

    /**
     * This thread runs during a connection with a remote device.
     * It handles all incoming and outgoing transmissions.
     */
    private class ConnectedTask extends AsyncTask<Void, String, Boolean> {

        private InputStream mInputStream = null;
        private OutputStream mOutputStream = null;
        private BluetoothSocket mBluetoothSocket = null;

        ConnectedTask(BluetoothSocket socket){

            mBluetoothSocket = socket;
            try {
                mInputStream = mBluetoothSocket.getInputStream();
                mOutputStream = mBluetoothSocket.getOutputStream();
            } catch (IOException e) {
                Log.e(TAG, "socket not created", e );
            }

            Log.d(TAG, "connected to " + mConnectedDeviceName);
            mConnectionStatus.setText("connected to " + mConnectedDeviceName);

            Toast.makeText(getApplication(), "Bluetooth: connected" , Toast.LENGTH_SHORT).show();
        }



        @Override
        protected Boolean doInBackground(Void... params) {

            byte [] readBuffer = new byte[1024];
            int readBufferPosition = 0;
            // Keep listening to the InputStream while connected
            while (true) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                if ( isCancelled() ) return false;

                try {
                    int bytesAvailable = mInputStream.available();

                    if(bytesAvailable > 0) {

                        byte[] packetBytes = new byte[bytesAvailable];
                        // Read from the InputStream
                        mInputStream.read(packetBytes);

                        for(int i=0;i<bytesAvailable;i++) {

                            byte b = packetBytes[i];
                            if(b == '\n')
                            {
                                byte[] encodedBytes = new byte[readBufferPosition];
                                System.arraycopy(readBuffer, 0, encodedBytes, 0,
                                        encodedBytes.length);
                                String recvMessage = new String(encodedBytes, "UTF-8");

                                readBufferPosition = 0;

                                Log.d(TAG, "recv message: " + recvMessage);
                                publishProgress(recvMessage);
                            }
                            else
                            {
                                readBuffer[readBufferPosition++] = b;
                            }
                        }
                    }
                } catch (IOException e) {

                    Log.e(TAG, "disconnected", e);
                    return false;
                }
            }

        }


        @Override
        protected void onProgressUpdate(String... recvMessage) {
            mConversationArrayAdapter.insert(mConnectedDeviceName + ": " + recvMessage[0], 0);

            String str;
            str = recvMessage[0];

            BMessage message = new BMessage(str);
            message.execute();
        }



        @Override
        protected void onPostExecute(Boolean isSucess) {
            super.onPostExecute(isSucess);

            if ( !isSucess ) {


                closeSocket();
                Log.d(TAG, "Device connection was lost");
                isConnectionError = true;
                showErrorDialog("Device connection was lost");
            }
        }

        @Override
        protected void onCancelled(Boolean aBoolean) {
            super.onCancelled(aBoolean);

            closeSocket();
        }

        void closeSocket(){

            try {

                mBluetoothSocket.close();
                Log.d(TAG, "close socket()");

            } catch (IOException e2) {

                Log.e(TAG, "unable to close() " +
                        " socket during connection failure", e2);
            }
        }

        void write(String msg){

            msg+= "\n";

            try {
                mOutputStream.write(msg.getBytes());
                mOutputStream.flush();
            } catch (IOException e) {
                Log.e(TAG, "Exception during send", e );
            }

        }
    }


    public void showPairedDevicesListDialog()
    {
        Set<BluetoothDevice> devices = mBluetoothAdapter.getBondedDevices();
        final BluetoothDevice[] pairedDevices = devices.toArray(new BluetoothDevice[0]);

        if ( pairedDevices.length == 0 ){
            showQuitDialog( "No devices have been paired.\n"
                    +"You must pair it with another device.");
            return;
        }

        String[] items;
        items = new String[pairedDevices.length];
        for (int i=0;i<pairedDevices.length;i++) {
            items[i] = pairedDevices[i].getName();
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Select device");
        builder.setCancelable(false);
        builder.setItems(items, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();

                // Attempt to connect to the device
                ConnectTask task = new ConnectTask(pairedDevices[which]);
                task.execute();
            }
        });
        builder.create().show();
    }



    public void showErrorDialog(String message)
    {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Quit");
        builder.setCancelable(false);
        builder.setMessage(message);
        builder.setPositiveButton("OK",  new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
                if ( isConnectionError  ) {
                    isConnectionError = false;
                    finish();
                }
            }
        });
        builder.create().show();
    }


    public void showQuitDialog(String message)
    {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Quit");
        builder.setCancelable(false);
        builder.setMessage(message);
        builder.setPositiveButton("OK",  new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
                finish();
            }
        });
        builder.create().show();
    }

    void sendMessage(String msg){

        if ( mConnectedTask != null ) {
            mConnectedTask.write(msg);
            Log.d(TAG, "send message: " + msg);
            mConversationArrayAdapter.insert("Me:  " + msg, 0);
        }
    }


    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {

        if(requestCode == REQUEST_BLUETOOTH_ENABLE){
            if (resultCode == RESULT_OK){
                //BlueTooth is now Enabled
                showPairedDevicesListDialog();
            }
            if( resultCode == RESULT_CANCELED){
                showQuitDialog( "You need to enable bluetooth");
            }
        }
        else if(requestCode == REQUEST_WIFI_SETTING){
            if (resultCode == RESULT_OK){
                BMessage message = new BMessage(
                        B_MESSAGE_ID.REQU_SET_WIFI,
                        B_RESPONSE_STATE.RESULT_OK,
                        data.getStringExtra("ssid"),
                        data.getStringExtra("pw")
                );

                mWifiStatus.setText("Waiting...");
                mSsidStatus.setText(" ");
                message.sendMsg();
            }
        }
    }


}