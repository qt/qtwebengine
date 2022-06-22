function FindProxyForURL(url, host)
{
   if (shExpMatch(host, "*.proxy1.com")) return "PROXY localhost:5551";
   if (shExpMatch(host, "*.proxy2.com")) return "PROXY localhost:5552";
   return "DIRECT";
}

