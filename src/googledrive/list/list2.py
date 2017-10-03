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
from apiclient import errors

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

# 접근 권한: https://developers.google.com/drive/v2/web/about-auth
SCOPES = 'https://www.googleapis.com/auth/drive' # all permision

CLIENT_SECRET_FILE = 'client_secret.json'
APPLICATION_NAME = 'Drive API Python Quickstart'
FOLDER = "application/vnd.google-apps.folder"  # 구글 드라이브 API에선 타입이 이 스트링인 파일을 폴더로 인식함
ROOT_FOLDER = "cloud_usb_test"  # 테스트를 위한 최상위 폴더

#구글 계정 권한에 대한 API
# quickstart.json -> .credentials 
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
    credentials = get_credentials()
    http = credentials.authorize(httplib2.Http())
    service = discovery.build('drive', 'v3', http=http)

    # === 17.01.31 ===#
    # API DOC: list() 에 들어가는 파라미터들(orderBy, q, fields 등)에 대한 문서 
    #   https://developers.google.com/resources/api-libraries/documentation/drive/v3/python/latest/index.html
    #
    # QUERY: list() 안에서 q="" 에 들어가는 쿼리문에 대한 문서
    #   https://developers.google.com/drive/v3/web/search-parameters
    #

    ## 1. 구글 드라이브 계정 내의 모든 파일, 디렉토리 정보를 다 가져옴
    all_files = []
    retrieve_all_files(service, all_files)

    # 2. 최상위 폴더부터 시작해서 모든 파일, 디렉토리 정보를 탐색
    result_files = []

    root_dir_id = find_id_by_name(all_files, ROOT_FOLDER)
    listing_files(root_dir_id, "", result_files, all_files)

    # 3. 탐색한 파일, 디렉토리 정보를 보여줌
    for file in result_files:
        print(file)

    # 4. 파일, 디렉토리 정보를 파일에 저장
    bridge = open(PIPE_PATH, "w")
    try:
        for file in result_files:
            bridge.write(file + "\n")
    finally:
        bridge.close()
        
def listing_files(folderID, directory, result_files, all_files):

    if not all_files:
        return        

    for item in all_files:
        if(('parents' in item) and (item['parents'][0]==folderID)):
            item['name'] = item['name'].replace(" ", "_")

            if item['mimeType'] == FOLDER:
                result_files.append('%s %s %s %s' % (directory + '/' + item['name'], "1", item['id'], "1"))
                listing_files(item['id'], directory + "/%s" % item['name'], result_files, all_files)
            else:
                result_files.append('%s %s %s %s' % (directory + '/' + item['name'], item['size'] ,item['id'], '0'))


def find_name_by_id(all_files, id):
    for item in all_files:
        if(item['id'] == id):
            return item['name']
    return False

def find_id_by_name(all_files, name):
    for item in all_files:
        if(item['name'] == name):
            return item['id']
    return False

def retrieve_all_files(service, all_files):
    pageToken = None
    i = 0
    while True:
        results = service.files().list(pageToken=pageToken,
            pageSize=1000,fields="nextPageToken, files(id, name, size, mimeType, parents)").execute()


        items = results.get('files', [])
        if not items:
            print('No files found.')
        else:
            print('Files:')
            for item in items:
                all_files.append(item)
                #print('{0} ({1})'.format(item['name'], item['id']))
        if(not results.get('nextPageToken')):
            break
      
        pageToken = results['nextPageToken']
        i = i+1
        
    print(i) 

if __name__ == '__main__':
    main()
