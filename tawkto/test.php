<?php

define("DEFAULT_URL", 'https://bitbucket.org/!api/2.0/snippets/tawkto/aA8zqE/4f62624a75da6d1b8dd7f70e53af8d36a1603910/files/webstats.json');

class Date {
  protected $dateTime = null;

  public function __construct(int $year, int $month, int $day) {
    $dateString = sprintf("%04d-%02d-%02d", $year, $month, $day);
    $this->dateTime = new DateTime($dateString);
    if ($this->dateTime == null) {
      error_log("Invalid date and time: $dateString");
    }
  }

  public function GetDateTime() {
    return $this->dateTime;
  }
}

class CTawkToTest {
  protected $url;

  public function __construct() {
    $this->url = DEFAULT_URL;
  }

  public function setUrl($newUrl) {
    $this->url = $newUrl;
  }

  public function processStatistics($fromDate = null, $toDate = null) {
    $fromString = "any datetime";
    $toString = "any datetime";
    if ($fromDate != null) {
      $fromString = $fromDate->GetDateTime()->format("Y-m-d");
    }
    if ($toDate != null) {
      $toString = $toDate->GetDateTime()->format("Y-m-d");
    }
    printf("\nRunning new report, from: %s to %s\n", $fromString, $toString);

    if ($fromDate === null && $toDate !== null) {
      error_log("fromDate is null; not quite as expected - the job will run without start date");
    } else if ($fromDate !== null && $toDate === null) {
      error_log("toDate is null; not quite as expected - the job will run without end date");
    } else if ($fromDate !== null && $toDate !== null) {
      // Sanity check
      if ($fromDate->GetDateTime() > $toDate->GetDateTime()) {
        error_log("from date is later than end date, switching those");
        $tmp = $fromDate;
        $fromDate = $toDate;
        $toDate = $tmp;
      }
      /* echo $fromDate->GetDateTime()->format('Y-m-d H:i:s');
      printf("\n");
      echo $toDate->GetDateTime()->format('Y-m-d H:i:s');
      printf("\n");
      $reportdays = intval($fromDate->GetDateTime()->diff($toDate->GetDateTime())->format("%a"));
      printf("Report days: %d\n", $reportdays);*/
    }

    $jsondata = file_get_contents($this->url);
    if ($jsondata === false) {
      error_log('Failed to open url: ' . $this->url);
      return false;
    }

    $json = json_decode($jsondata);
    if (json_last_error() != JSON_ERROR_NONE) {
      error_log('Failed to decode json file: ' . $this->url . '   error code: ' . json_last_error());
      return false;
    }


    usort($json, function($d1, $d2) { // Sort ascending
      return new DateTime($d1->date) > new DateTime($d2->date) ? 1 : -1;
    });

    $outData = array();
    for ($i = 0; $i < count($json); $i++) {
      $dtime = new DateTime($json[$i]->date); // Current record
      if ($fromDate !== null) {
        if ($dtime < $fromDate->GetDateTime()) {
          continue; // Earlier than what is needed
        }
      }
      if ($toDate !== null) {
        if ($dtime > $toDate->GetDateTime()) {
          break;  // It has been sorted asc so there is no point seeking any further
        }
      }

      if (!is_numeric($json[$i]->chats) || !is_numeric($json[$i]->missedChats)) {
        error_log("Invalid record skipped as chats/missedChats found to be not numeric: $json[$i]->websiteId / $json[$i]->date");
      } else {
        if (array_key_exists($json[$i]->websiteId, $outData)) {
          $outData[$json[$i]->websiteId][0] += $json[$i]->chats;
          $outData[$json[$i]->websiteId][1] += $json[$i]->missedChats;
        } else {
          $outData[$json[$i]->websiteId] = array($json[$i]->chats, $json[$i]->missedChats);
        }
      }
    }

    if (count($outData) < 1) {
      $fromString = $fromDate->GetDateTime()->format('Y-m-d'); //  H:i:s
      $toString = $toDate->GetDateTime()->format('Y-m-d'); //  H:i:s
      error_log("No data found for the following conditions: start date: $fromString, to date: $toString");
      return false;
    }

    ksort($outData);

    // json_encode could be used with a properly formed array if output would be needed; now just print it
    // as consumer script will most likely redirect that anyways; specs did not speak about it usage in this test
    printf("[");
    $keys = array_keys($outData);
    for ($i = 0; $i < count($keys); $i++) {
      printf("{\n");
      printf("\t\"websiteId\": \"$keys[$i]\",\n");
      printf("\t\"chats\": ".$outData[$keys[$i]][0].",\n");
      printf("\t\"missedChats\": ".$outData[$keys[$i]][1]."\n");
      printf("}");
      if ($i + 1 < count($keys)) {
        printf(", ");
      }
    }
    printf("]");
    printf("\n");
    return true;
  }
}

$ttt = new CTawkToTest;
//$ttt->setUrl("input.json");
$ttt->processStatistics();
$ttt->processStatistics(new Date(2019, 3, 1), new Date(2019, 2, 1));
$ttt->processStatistics(new Date(2019, 4, 3), new Date(2019, 4, 11));

?>

