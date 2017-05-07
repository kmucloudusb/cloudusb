# -*- coding: utf-8 -*-

# To start this python script,
# You need "client_secret.json" which contains your Google Drive personal data.
# It can be downloaded at "https://developers.google.com/drive/v3/web/quickstart/python".

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

    # API DOC: list() 에 들어가는 파라미터들(orderBy, q, fields 등)에 대한 문서 
    #   https://developers.google.com/resources/api-libraries/documentation/drive/v3/python/latest/index.html
    #
    # QUERY: list() 안에서 q="" 에 들어가는 쿼리문에 대한 문서
    #   https://developers.google.com/drive/v3/web/search-parameters
    #

    ## 파일 이름으로 파일 아이디 찾기 
    # 주의: 다른 폴더 같은 이름도 다 찾으므로, 지금은 이름 다르게 해야함
    # target_file_name = "movie.wmv"
    # target_file = service.files().list(
    #     q=("name = '%s'" % target_file_name)).execute()
    # if not target_file:
    #     print(target_file_name + ": not found")
    #     return
    # target_file_id = target_file.get('files')[0].get('id')
    # print("id: "+target_file_id)


    ## 파일 다운로드
    down_file_id = target_file_id '0B8CPvjgKUMvtM2tTWTctVzNrUm8'
    #down_file_name = 'downloaded_file'
    #down_byte_begin = 0
    #down_byte_end = 1787913

    ##1. 전체 파일 다운
    #download_file(service, down_file_id, down_file_name) 

    ##2. 전체 파일 나눠서 다운 -> 이거로 합시다
    GD_download_file(service, down_file_id) 

    ##3. 일부분만 다운 (아직 크기 작은 파일만 됨)
    #partial_download(service, down_file_id, down_byte_begin, down_byte_end) 


def listing_files(service, folderID, directory, result_files, result_directories):
    result_directories.append(directory)

    results = service.files().list(
        orderBy="createdTime",
        q=("'%s' in parents" % folderID),
        fields="files(id, name, mimeType, size)").execute()
    items = results.get('files', [])
    if not items:
        # result_files.append('%s : No files found.'%directory)
        pass
    else:
        for item in items:
            if item['mimeType'] == FOLDER:
                listing_files(service, item['id'], directory + "/%s" % item['name'], result_files, result_directories)
            else:
                result_files.append('%s %s' % (directory + '/' + item['name'], item['id'], itme['size']))


##==================================================================##
def download_file(drive_service, file_id, file_name):
    file_id = '0B8CPvjgKUMvtYklyZU1yMGJrbms'
    file_name = "uhahhaha.mp4"

    request = drive_service.files().get_media(fileId=file_id)
    fh = io.FileIO(file_name, 'wb')
    downloader = MediaIoBaseDownload(fh, request)
    done = False
    while done is False:
        status, done = downloader.next_chunk()
        print("Download %d%%." % int(status.progress() * 100))


def partial(total_byte_len, part_size_limit):
    s = []
    for p in range(0, total_byte_len, part_size_limit):
        last = min(total_byte_len - 1, p + part_size_limit - 1)
        s.append([p, last])
    return s

def partial_download(service, file_id, byte_begin, byte_end):
    drive_file = service.files().get(fileId=file_id, fields='size, id, originalFilename').execute()

    print(drive_file)

    download_url = service.files().get_media(fileId=file_id).uri

    total_size = int(drive_file.get('size'))
    originalFilename = drive_file.get('originalFilename')
    filename = './' + originalFilename
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
            print(str(byte_end - byte_begin)+"...")
        return filename
    else:
        return None  

def GD_download_file(service, file_id):
  drive_file = service.files().get(fileId=file_id, fields='size, id, originalFilename').execute()

  print(drive_file)

  download_url = service.files().get_media(fileId=file_id).uri


  total_size = int(drive_file.get('size'))
  s = partial(total_size, 100000000) # I'm downloading BIG files, so 100M chunk size is fine for me
  originalFilename = drive_file.get('originalFilename')
  filename = './' + originalFilename
  if download_url:
      with open(filename, 'wb') as file:
        print("Bytes downloaded: ")
        for bytes in s:
          headers = {"Range" : 'bytes=%s-%s' % (bytes[0], bytes[1])}
          resp, content = service._http.request(download_url, headers=headers)
          if resp.status == 206 :
                file.write(content)
                file.flush()
          else:
            print('An error occurred: %s' % resp)
            return None
          print(str(bytes[1])+"...")
      return filename
  else:
    return None          


if __name__ == '__main__':
    main()
