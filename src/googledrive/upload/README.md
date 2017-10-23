# Google Drive API를 이용한 파일 업로드

## 기능(구현 10%)
구글 드라이브에 파일을 업로드한다.

../upload(current) 폴더에 있는 파일 이름이 fname인 파일을 구름USB 루트디렉토리에 업로드한다.(개선 필요)
그리고 fifo에 해당 파일의 구글 드라이브 fid를 쓴다.

## 사용

```
$ python upload.py --fname FILE_ID --fifo FIFO_PATH
```

## 예제
```
$ python upload.py --fname 0B8CPvjgKUMvtWVVtY3ZKLWNTb2M --fifo ../myfifo
```

file_id가 "0B8CPvjgKUMvtWVVtY3ZKLWNTb2M" 인 파일을 업로드한다.

# 참고

1. 구글 계정 접근 권한을 바꿔야함.(readonly -> all)
2. 구글 드라이브에 100% 동일한 파일이 있어도 업로드 됨
3. 폴더 업로드(하위 파일 업로드) 기능은 구현해야 함
