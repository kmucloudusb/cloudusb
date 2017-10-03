# Google Drive API를 이용한 파일 리스트 다운로드

## 기능
구글 드라이브 계정에서 구름USB 루트디렉토리 하위에 있는 모든 파일 목록을 다운로드 받는다.
단, 휴지통에 있는 파일 제외
../myfifo에 파일 목록을 쓴다

## 사용
```
$ python list.py
```

## 결과

file_full_path, file_size, file_id, is_folder

```
...
/두번째폴더/두번째안에두번째 1 0B8CPvjgKUMvtRFFBQ3M5VzBGSUk 1
/두번째폴더/두번째안에첫번째 1 0B8CPvjgKUMvtM0xPaXNucGxIazg 1
/첫번째폴더 1 0B8CPvjgKUMvtcGVscHJyWWpDREU 1
/첫번째폴더/photo.jpg 9094 0B8CPvjgKUMvtVko2bTlkMm1QTGM 0
...
```



# 참고

## 방식 1(list_recursive.py)
폴더 한번 당 리컬시브로 서치 쿼리 보내기

## 방식 2(현재 이 방법 사용 중)
모든 파일 다 가져와서 로컬에서 리컬시브 처리

## 관련 이슈 링크
https://github.com/kmucloudusb/cloudusb/issues/6
