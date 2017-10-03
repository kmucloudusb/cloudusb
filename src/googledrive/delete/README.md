# Google Drive API를 이용한 파일 삭제

## 기능
구글 드라이브에 있는 파일을 영구적으로 삭제
폴더를 삭제할 경우, 하위 폴더/파일을 모두 영구적으로 삭제

## 사용
```
$ python delete.py --fid FILE_ID
```

## 예제
```
$ python delete.py --fid 0B8CPvjgKUMvtWVVtY3ZKLWNTb2M
```

file_id가 "0B8CPvjgKUMvtWVVtY3ZKLWNTb2M" 인 파일을 삭제한다.