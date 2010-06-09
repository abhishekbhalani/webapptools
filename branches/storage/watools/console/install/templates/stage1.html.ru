<html>
<head>
    <title>Установка Web "A" Tools</title>
</head>
<body>
    <h1>Welcome</h1>
    <div>Этот мастер проведет Вас через процесс установки.</div>
    <form action="index.php" method="POST">
        <table border="0">
            <tr>
                <td>Выберите язык </td>
                <td><select name="lang">[{html_options values=$langs output=$lnNames selected=$defLang}]</select></td>
            </tr>
            <tr>
                <td>&nbsp;</td>
                <td><input type="submit" value="Далее >"></td>
            </tr>
        </table>
    </form>
</body>
</html>