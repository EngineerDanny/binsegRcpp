# You can modify the PR comment header here. You can use github markdown e.g.
# emojis like :tada:.

glue::glue(
  "The table below represents the test results generated",
  " after running `",
  test_function,
  "` function",
  " on this PR branch",
  "\n",
  "<br/> <br/>",
  image_url
)
