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
from apiclient.http import MediaFileUpload

file_id = ''

try:
    import argparse

    tools.argparser.add_argument('--fid', default='-1', help='file id')

    flags = argparse.ArgumentParser(parents=[tools.argparser]).parse_args()

    if flags.fid:
        file_id = flags.fid

except ImportError:
    flags = None

reload(sys)
sys.setdefaultencoding('utf-8')

# If modifying these scopes, delete your previously saved credentials
# at ~/.credentials/drive-python-quickstart.json
# 접근 권한: https://developers.google.com/drive/v2/web/about-auth
SCOPES = 'https://www.googleapis.com/auth/drive' # all permision

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

    file_upload(service)

def file_upload(service):
    file_metadata = { 'name' : 'photo.jpg' }
    media = MediaFileUpload('photo.jpg',
                            mimetype='image/jpeg')
    file = service.files().create(body=file_metadata,
                                        media_body=media,
                                        fields='id').execute()
    print('File ID: %s' % file.get('id'))

if __name__ == '__main__':
    main()
