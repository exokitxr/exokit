IF EXIST ".\node\node.exe" (
  .\node_modules\isolator\lib\windows\isolator.exe -- .\node\node.exe . %*
) ELSE (
  .\node_modules\isolator\lib\windows\isolator.exe -- node . %*
)
