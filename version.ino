int getVersionMajorRelease()
{
  return 0;
}
int getVersionMinorRelease()
{
  return 1;
}
int getVersionMajorBuild()
{
  return 0;
}
int getVersionMinorBuild()
{
  return 7;
}

String getVersion()
{
  return String(getVersionMajorRelease())+'.'+String(getVersionMinorRelease())+'.'+String(getVersionMajorBuild())+'.'+String(getVersionMinorBuild());
}
