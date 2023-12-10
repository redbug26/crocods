<?php

$action = "browse";
if ($_REQUEST["action"] != "") {
    $action = $_REQUEST["action"];
}

function addString($str)
{
    $str = str_replace("\n", "", $str);
    $str = str_replace("\r", "", $str);
    echo $str;
    echo pack("C", 0);
}

$bdd = 1;

if ($bdd == 1) {
    $base_url = "https://www.cpc-power.com/_javacpc_markus/";
} else if ($bdd == 2) {
    $base_url = "http://cpc.devilmarkus.de/games/";
}

if ($action == "browse") {
    $file = $bdd . ".xml";

    if (file_exists($file)) {
        $data = file_get_contents($file);
    } else {

        $url = $base_url . "crocods.php?action=detailist";

        $curl = curl_init();
        curl_setopt($curl, CURLOPT_URL, $url);
        curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($curl, CURLOPT_HEADER, false);
        curl_setopt($curl, CURLOPT_USERAGENT, "BDDBrowser/2.9.7c Java/1.8.0_192");
        $data = curl_exec($curl);
        curl_close($curl);
        // print_r($data);exit;

        $data = strstr($data, "<games>");

        file_put_contents($file, $data);
    }

    // $data = str_replace("&", "&amp;", $data);
    // $data = str_replace("png\">", "png\"/>", $data);

    $xml = simplexml_load_string($data);

    echo "KYCPC";

    $count = 0;

    foreach ($xml->game as $game) {

        unset($media_row);
        foreach ($game->media as $media) {
            $media_row[strval($media->attributes()->type)] = strval($media->attributes()->id);
        }

        if (array_key_exists("Disquette", $media_row)) {
            $count++;
        }
    }

    echo pack("v", $count); // entier court non signé (toujours 16 bits, ordre des octets little endian)

    foreach ($xml->game as $game) {
        $title = strval($game->attributes()->title);

        unset($media_row);
        foreach ($game->media as $media) {
            $media_row[strval($media->attributes()->type)] = strval($media->attributes()->id);
        }

        // print_r($media_row);

        if (array_key_exists("Disquette", $media_row)) {
            addString($title);
            addString($media_row["Disquette"]);
            addString($media_row["Screenshot"]);
        }

        // print_r($media_row);
    }
    // echo $data;
}

if ($action == "get") {

    $url = $base_url . "crocods.php?action=get&id=" . $_REQUEST["id"];

    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($curl, CURLOPT_HEADER, false);
    curl_setopt($curl, CURLOPT_USERAGENT, "BDDBrowser/2.9.7c Java/1.8.0_192");

    $data = curl_exec($curl);
    curl_close($curl);

    print_r($data);

}

if ($action == "cpcpower") {

    $file = $bdd . ".xml";

    if (file_exists($file)) {
        $data = file_get_contents($file);
    } else {

        $url = $base_url . "crocods.php?action=detailist";

        $curl = curl_init();
        curl_setopt($curl, CURLOPT_URL, $url);
        curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($curl, CURLOPT_HEADER, false);
        curl_setopt($curl, CURLOPT_USERAGENT, "BDDBrowser/2.9.7c Java/1.8.0_192");
        $data = curl_exec($curl);
        curl_close($curl);
        // print_r($data);exit;

        $data = strstr($data, "<games>");

        file_put_contents($file, $data);
    }

    // $data = str_replace("&", "&amp;", $data);
    // $data = str_replace("png\">", "png\"/>", $data);

    $xml = simplexml_load_string($data);

    $count = 0;

    foreach ($xml->game as $game) {
        $title = strval($game->attributes()->title);

        unset($id);
        unset($media_row);
        foreach ($game->media as $media) {
            if (strval($media->attributes()->type) == "screenshot") {
                $id = substr(strstr(strval($media->attributes()->id), "_"), 1);
            }
            $media_row[strval($media->attributes()->type)] = strval($media->attributes()->id);
        }

        // echo $id;
        // // echo "(" . strval($game->attributes()->id) . "<br>";
        // print_r($media_row);
        // exit;

        // screen0_
        if ($_REQUEST["id"] == $id) {
            $cpcpower_id = $media_row["Disquette"];
        }

        // if (array_key_exists("Disquette", $media_row)) {
        //     addString($title);
        //     addString($media_row["Disquette"]);
        //     addString($media_row["Screenshot"]);
        // }
    }
    // echo $data;

    $url = $base_url . "crocods.php?action=get&id=" . $cpcpower_id;

    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($curl, CURLOPT_HEADER, false);
    curl_setopt($curl, CURLOPT_USERAGENT, "BDDBrowser/2.9.7c Java/1.8.0_192");

    $data = curl_exec($curl);
    curl_close($curl);

    print_r($data);

}
