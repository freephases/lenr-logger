int getVersionMajorRelease()
{
  return 0;
}
int getVersionMinorRelease()
{
  return 2;
}
int getVersionMajorBuild()
{
  return 1;
}
int getVersionMinorBuild()
{
  return 1;
}

String getVersion()
{
  return String(getVersionMajorRelease())+'.'+String(getVersionMinorRelease())+'.'+String(getVersionMajorBuild())+'.'+String(getVersionMinorBuild());
}
