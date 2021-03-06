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

file_name = ''
pipe_path= ''

try:
    import argparse

    tools.argparser.add_argument('--fifo', default='./../myfifo', help='pipe path')
    tools.argparser.add_argument('--fname', default='-1', help='file name IN current directory')

    flags = argparse.ArgumentParser(parents=[tools.argparser]).parse_args()

    if flags.fname:
        file_name = flags.fname

    if flags.fifo:
        pipe_path = flags.fifo

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
FOLDER_TYPE = "application/vnd.google-apps.folder"  # 구글 드라이브 API에선 타입이 이 스트링인 파일을 폴더로 인식함
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

    # 1. ROOT_DIRECTORY 이름을 가진 최상위 폴더를 찾음
    first_folder = service.files().list(
        q=("mimeType = '%s' and name = '%s'" %(FOLDER_TYPE, ROOT_FOLDER))).execute()
    first_folder_item = first_folder.get('files', [])
    root_dir_id = 0
    if not first_folder_item:
        print('No %s found.' % ROOT_FOLDER)
    else:
        for item in first_folder_item:
            root_dir_id = item['id']

    file_path = "./"
    uploaded_id = file_upload(service, root_dir_id, file_path, file_name)

    # 파일, 디렉토리 정보를 파일에 저장
    bridge = open(pipe_path, "w")
    try:
        bridge.write(uploaded_id + "\n")
    finally:
        bridge.close()


def file_upload(service, root_dir_id, file_path, file_name):

    prev_path = [root_dir_id]
    file_metadata = {
    'name' : file_name,
    'parents' : prev_path }

    file_name = file_name;
    file_full_path = file_path + file_name;

    media = MediaFileUpload(file_full_path)
    #media = MediaFileUpload('photo.jpg', mimetype='image/jpeg')

    file = service.files().create(body=file_metadata,
                                        media_body=media,
                                        fields='id').execute()
    print('File Upload Success: %s' % file.get('id'))

    return file.get('id')

if __name__ == '__main__':
    main()
