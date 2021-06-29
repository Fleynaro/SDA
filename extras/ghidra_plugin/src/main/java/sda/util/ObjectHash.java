package sda.util;

public class ObjectHash
{
    private String hashContent = "";
    private Long hash = 0L;

    public ObjectHash() {
    }

    public ObjectHash(Long hash)
    {
        this.hash = hash;
    }

    public ObjectHash(Long hash, String hashContent)
    {
        this.hash = hash;
        this.hashContent = hashContent;
    }

    public void addValue(String value) {
        hashContent += "{" + value + "}";
    }

    public void addValue(int value) {
        addValue((long)value);
    }

    public void addValue(Long value) {
        addValue(value.toString());
    }

    public long getHash() {
        return hash*31 + hash(hashContent);
    }

    public void join(ObjectHash hash) {
        this.hash = this.hash * 31 + hash.getHash();
    }
    public void add(ObjectHash hash) {
        this.hash = this.hash + hash.getHash();
    }

    private static long hash(String string) {
        long h = 1125899906842597L;
        for (int i = 0; i < string.length(); i++) {
            h = 31*h + string.charAt(i);
        }
        return h;
    }
}
