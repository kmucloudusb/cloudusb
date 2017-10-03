# Google Drive API를 이용한 파일 업로드

## 기능
구글 드라이브에 있는 파일 하나를 다운로드한다.
파일 이름은 FILE_ID(확장자 없음)
위치는 download 폴더(추후 변경 예정)

## 사용

```
$ python download.py --fid FILE_ID
```

## 예제
```
$ python download.py --fid 0B8CPvjgKUMvtWVVtY3ZKLWNTb2M
```

file_id가 "0B8CPvjgKUMvtWVVtY3ZKLWNTb2M" 인 파일을 다운로드한다.