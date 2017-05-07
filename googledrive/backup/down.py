# -*- coding: utf-8 -*-
from __future__ import print_function
import httplib2
import os
import sys
import errno
import io

from apiclient import discovery
from oauth2client import client
from oauth2client import tools
from oauth2client.file import Storage
from apiclient.http import MediaIoBaseDownload


PIPE_PATH = ''

try:
    import argparse

    tools.argparser.add_argument('--path', default='./../myfifo', help='pipe path')

    flags = argparse.ArgumentParser(parents=[tools.argparser]).parse_args()

    if flags.path:
        PIPE_PATH = flags.path

except ImportError:
    flags = None

reload(sys)
sys.setdefaultencoding('utf-8')

# If modifying these scopes, delete your previously saved credentials
# at ~/.credentials/drive-python-quickstart.json
# 접근 권한: https://developers.google.com/drive/v2/web/about-auth
SCOPES = 'https://www.googleapis.com/auth/drive.readonly'

CLIENT_SECRET_FILE = 'client_secret.json'
APPLICATION_NAME = 'Drive API Python Quickstart'
FOLDER = "application/vnd.google-apps.folder"  # 구글 드라이브 API에선 타입이 이 스트링인 파일을 폴더로 인식함
ROOT_FOLDER = "cloud_usb_test"  # 테스트를 위한 최상위 폴더


def get_credentials():
    home_dir = os.path.expanduser('~')
    credential_dir = os.path.join(home_dir, '.credentials')
    if not os.path.exists(credential_dir):
        os.makedirs(credential_dir)
    credential_path = os.path.join(credential_dir,
                                   'drive-python-quickstart.json')

    store = Storage(credential_path)
    credentials = store.get()
    if not credentials or credentials.invalid:
        flow = client.flow_from_clientsecrets(CLIENT_SECRET_FILE, SCOPES)
        flow.user_agent = APPLICATION_NAME
        if flags:
            credentials = tools.run_flow(flow, store, flags)
        else:  # Needed only for compatibility with Python 2.6
            credentials = tools.run(flow, store)
        print('Storing credentials to ' + credential_path)
    return credentials


def main():
    # 구글 계정 권한 얻기
    credentials = get_credentials()
    http = credentials.authorize(httplib2.Http())
    service = discovery.build('drive', 'v3', http=http)

    ## 파일 다운로드

    # 파일 id
    file_id = '0B8CPvjgKUMvtVGxGZHBCaWVabVk'

    # 파일 size
    drive_file = service.files().get(fileId=file_id, fields='size').execute()
    byte_begin = 0
    byte_end = int(drive_file.get('size'))

    file_path = './'

    partial_download(service, file_id, byte_begin, byte_end, file_path)

def partial_download(service, file_id, byte_begin, byte_end, file_path):
    drive_file = service.files().get(fileId=file_id, fields='size, id, originalFilename').execute()

    download_url = service.files().get_media(fileId=file_id).uri

    total_size = int(drive_file.get('size'))
    originalFilename = drive_file.get('originalFilename')
    filename = file_path + originalFilename
    if download_url:
        with open(filename, 'wb') as file:
            print("Bytes downloaded: ")
            headers = {"Range" : 'bytes=%s-%s' % (byte_begin, byte_end)}
            resp, content = service._http.request(download_url, headers=headers)
            if resp.status == 206 :
                file.write(content)
                file.flush()
            else:
                print('An error occurred: %s' % resp)
                return None
            print(str(byte_end - byte_begin)+"Bytes Success!")
        return filename
    else:
        return None  

if __name__ == '__main__':
    main()
