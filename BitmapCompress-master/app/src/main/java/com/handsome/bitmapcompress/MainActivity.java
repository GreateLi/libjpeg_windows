package com.handsome.bitmapcompress;

import android.Manifest;
import android.annotation.TargetApi;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.provider.MediaStore;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class MainActivity extends AppCompatActivity {

    private static final int REQUEST_PICK_IMAGE = 0x0011;
    private static final int REQUEST_KITKAT_PICK_IMAGE = 0x0012;
    private static final int REQUEST_PERMISSION_CODE = 0x0013;

    String dd = "{\"appKey\":\"_8I4wKtIaH92T9bRL9njpD1VuDN6OdDn\",\"businessType\":\"official\",\"contents\":[{\"algorithmVersion\":\"V2.2.20181224\",\"expirationTime\":1577750400000,\"snRange\":\"IMIH-A200M40V1CO000025\",\"usage\":\"offline\"}],\"flag\":\"hjimi\",\"isValid\":true,\"localSN\":\"\",\"platform\":\"android\",\"sequence\":\"1545896379077\",\"timestamp\":1545896379077,\"version\":\"1.0\"}";
    String snStr = "{\"appKey\":\"_8I4wKtIaH92T9bRL9njpD1VuDN6OdDn\",\"businessType\":\"official\",\"contents\":[{\"algorithmVersion\":\"V2.2.20181224\",\"expirationTime\":1577750400000,\"snRange\":\"IMIH-A200M40V1CO000025\",\"usage\":\"offline\"},{\"algorithmVersion\":\"V2.2.20181230\",\"expirationTime\":1577750400000,\"snRange\":\"IMIH-A200M40V1CO000030\",\"usage\":\"offline\"}],\"flag\":\"hjimi\",\"isValid\":true,\"localSN\":\"\",\"platform\":\"android\",\"sequence\":\"1545896379077\",\"timestamp\":1545896379077,\"version\":\"1.0\"}";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    /**
     * 点击事件
     *
     * @param v
     */
    public void pickFromGallery(View v) {
        checkPermission();
    }

    public void depthCompress(View view)
    {
        String opath =   Environment.getExternalStorageDirectory().getAbsolutePath()+"/hjimi/dep_1.bin";
        //String opath =   Environment.getExternalStorageDirectory().getAbsolutePath()+"/test/dep.jpg";
        String outpath =   Environment.getExternalStorageDirectory().getAbsolutePath()+"/test/outdep.jpg";

        File ofile = new File(opath);
        File ooutfile = new File(outpath);

        FileInputStream inputStream;
        try {
            inputStream =  new FileInputStream(ofile);

            int flen = inputStream.available();
            byte rgbByte[] = new byte[flen];
            int len   = inputStream.read(rgbByte);
            if(len ==flen)
            {
                inputStream.close();
            }


            final byte[] depthBuffer = ImageUtils.dethRGB2Buffer(rgbByte, 640, 480);
            Log.e("depthcompress","start:"+System.currentTimeMillis());
            byte [] outData = CompressUtils.compressByLibjpeg(depthBuffer,100,2,640,480 );
            Log.e("GGGG22222222","start:"+System.currentTimeMillis());
            if(null != outData)
            saveFile(ooutfile,outData);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    public void colorCompress(View view)
    {
        String opath =   Environment.getExternalStorageDirectory().getAbsolutePath()+"/hjimi/rgb_1.bin";
      //  String opath =   Environment.getExternalStorageDirectory().getAbsolutePath()+"/test/rgb.jpg";
        String outpath =   Environment.getExternalStorageDirectory().getAbsolutePath()+"/test/outrgb.jpg";

        File ofile = new File(opath);
        File ooutfile = new File(outpath);

        FileInputStream inputStream;
        try {
            inputStream =  new FileInputStream(ofile);

            int flen = inputStream.available();
            byte rgbByte[] = new byte[flen];
            int len   = inputStream.read(rgbByte);
            if(len ==flen)
            {
                inputStream.close();
            }
            final byte []  rgbBuffer = ImageUtils.getRGBBuffer(rgbByte, 640, 480);
            Log.e("depthcompress","start:"+System.currentTimeMillis());
            byte [] outData = CompressUtils.compressByLibjpeg(rgbBuffer,25,0,640,480 );
            Log.e("GGGG22222222","start:"+System.currentTimeMillis());
            if(null != outData)
            saveFile(ooutfile,outData);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * 申请权限
     */
    public void checkPermission() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    REQUEST_PERMISSION_CODE);
        } else {
            selectFile();
        }
    }

    /**
     * 选择文件
     */
    public void selectFile() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
            startActivityForResult(new Intent(Intent.ACTION_GET_CONTENT).setType("image/*"), REQUEST_PICK_IMAGE);
        } else {
            Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
            intent.addCategory(Intent.CATEGORY_OPENABLE);
            intent.setType("image/*");
            startActivityForResult(intent, REQUEST_KITKAT_PICK_IMAGE);
        }
    }

    /**
     * 返回结果
     *
     * @param requestCode
     * @param resultCode
     * @param data
     */
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode == RESULT_OK) {
            switch (requestCode) {
                case REQUEST_PICK_IMAGE:
                    if (data != null) {
                        Uri uri = data.getData();
                        compressImage(uri);
                    }
                    break;
                case REQUEST_KITKAT_PICK_IMAGE:
                    if (data != null) {
                        Uri uri = ensureUriPermission(this, data);
                        compressImage(uri);
                    }
                    break;
            }
        }
    }

    /**
     * 分配临时权限，仅适配机型作用
     *
     * @param context
     * @param intent
     * @return
     */
    @SuppressWarnings("ResourceType")
    @TargetApi(Build.VERSION_CODES.KITKAT)
    public static Uri ensureUriPermission(Context context, Intent intent) {
        Uri uri = intent.getData();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            final int takeFlags = intent.getFlags() & Intent.FLAG_GRANT_READ_URI_PERMISSION;
            context.getContentResolver().takePersistableUriPermission(uri, takeFlags);
        }
        return uri;
    }

    /**
     * 压缩图片
     * 注意：记得手动开启权限
     *
     * @param uri
     */
    public void compressImage(Uri uri) {
        try {
            File saveFile = new File(getExternalCacheDir(), "NDK压缩.jpg");
            Bitmap bitmap = MediaStore.Images.Media.getBitmap(getContentResolver(), uri);
            int code = CompressUtils.compressBitmap(bitmap, 20, saveFile.getAbsolutePath().getBytes(), true);

            File saveFile1 = new File(getExternalCacheDir(), "质量压缩.jpg");
            CompressUtils.compressQuality(bitmap, 20, saveFile1);

            File saveFile2 = new File(getExternalCacheDir(), "尺寸压缩.jpg");
            CompressUtils.compressSize(bitmap, saveFile2);

            //采样率比较特殊，需要传递文件的目录，这里采用直接指定目录的文件
            File saveFile3 = new File(getExternalCacheDir(), "采样率压缩.jpg");
            File LocalFile = new File("/storage/emulated/0/DCIM/Camera/IMG_20171216_171956.jpg");
            if (LocalFile.exists()) {
                CompressUtils.compressSample(LocalFile.getAbsolutePath(), saveFile3);
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void saveFile(File file , byte[] data)
    {
        try {
            FileOutputStream fos = new FileOutputStream(file);
            fos.write(data);
            fos.flush();
            fos.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    /**
     * 权限回调
     *
     * @param requestCode
     * @param permissions
     * @param grantResults
     */
    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if (requestCode == REQUEST_PERMISSION_CODE) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                selectFile();
            } else {
                Toast.makeText(MainActivity.this, "Permission Denied", Toast.LENGTH_SHORT).show();
            }
            return;
        }
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }
}
