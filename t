stderrstderrstderrstderrstderrstderrstderrstderrstderrstderrstderrstderrstderrstderrstderrstderrstderrstderrstderrstderrstderrstderrstderr# 1
    echo a \
  ;
    echo b
# 2
  echo hi>>y
# 3
  exec echo c
# 4
  (
   cat<file
  )>>C
# 5
  echo abc def
# 6
  (
   echo abc def
  )
# 7
    echo file \
  |
    wc -l
# 8
    (
       echo z \
     ||
       echo b
    ) \
  &&
      echo a \
    |
      sort \
    |
      wc -l
